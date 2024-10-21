#define _GNU_SOURCE
#include "memory_manager.h"

typedef struct memory_block {
    void *start;
    void *end;
} memory_block;

typedef struct dynamic_array_head {
    memory_block* array;
    size_t total_size;
    size_t current_size;
} dynamic_array_head;

pthread_rwlock_t rwlock;
void* memory_;
size_t size_;

dynamic_array_head head;

bool resize(size_t new_size) {
    if (head.total_size < new_size) {
        void* temp = realloc(head.array, new_size * sizeof(memory_block));
        if (!temp) return false;
        head.array = temp;
        head.total_size = new_size;
    } else {
        if (head.current_size < new_size) return false;
        void* temp = realloc(head.array, new_size * sizeof(memory_block));
        if (!temp) return false;
        head.array = temp;
        head.total_size = new_size;
    }
    head.total_size = new_size;
    return true;
}

void shove_left_(size_t index) {
    while ((index + 1) < head.total_size) {
        head.array[index] = head.array[index + 1];
        index++;
    }
}

void shove_right_(size_t index) {
    size_t walker = head.total_size - 1;
    while (walker != index) {
        head.array[walker] = head.array[walker - 1];
        walker--;
    }
}

void mem_init(size_t memory_size) {
    size_t array_size = memory_size / 64;
    head.array = malloc(array_size * sizeof(memory_block));
    head.current_size = 0;
    head.total_size = array_size;
    for (size_t i = 0; i < head.total_size; i++) {
        head.array[i] = (memory_block){NULL, NULL};
    }
    memory_ = malloc(memory_size);
    size_ = memory_size;
    pthread_rwlock_init(&rwlock, NULL);
}

void* mem_alloc(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;
    pthread_rwlock_wrlock(&rwlock);
    if (head.current_size == head.total_size) {
        resize(head.total_size * 2);
    }
    if (head.current_size == 0) {
        head.array[head.current_size++] =
            (memory_block){memory_, memory_ + size};

        pthread_rwlock_unlock(&rwlock);
        return memory_;
    }
    void* previous_end = memory_;
    for (size_t i = 0; i < head.current_size; i++) {
        if (head.array[i].start == NULL || head.array[i].end == NULL)
            if ((head.array[i].start - previous_end) >= size) {
                head.current_size++;
                shove_right_(i);
                head.array[i] =
                    (memory_block){previous_end, previous_end + size};

                pthread_rwlock_unlock(&rwlock);
                return previous_end;
            }
        previous_end = head.array[i].end;
    }
    if ((memory_ + size_ - previous_end) >= size) {
        head.array[head.current_size++] =
            (memory_block){previous_end, previous_end + size};
        pthread_rwlock_unlock(&rwlock);
        return previous_end;
    }
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void* DA_add_mem_block__nolock__(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;
    if (head.current_size == head.total_size) {
        resize(head.total_size * 2);
    }
    if (head.current_size == 0) {
        head.array[head.current_size++] =
            (memory_block){memory_, memory_ + size};
        return memory_;
    }
    void* previous_end = memory_;
    for (size_t i = 0; i < head.current_size; i++) {
        if (head.array[i].start == NULL || head.array[i].end == NULL)
            if ((head.array[i].start - previous_end) >= size) {
                head.current_size++;
                shove_right_(i);
                head.array[i] =
                    (memory_block){previous_end, previous_end + size};
                return previous_end;
            }
        previous_end = head.array[i].end;
    }
    if ((memory_ + size_ - previous_end) >= size) {
        head.array[head.current_size++] =
            (memory_block){previous_end, previous_end + size};
        return previous_end;
    }
    return NULL;
}

void* mem_resize(void* block, size_t size) {
    if (size > size_) return NULL;
    if (size == 0) {
        mem_free(block);
        return NULL;
    }
    pthread_rwlock_wrlock(&rwlock);
    for (size_t i = 0; i < head.current_size;
         i++) {  // make binary search u dumb fuck
        if (head.array[i].start == block) {
            size_t available_space_after =
                (i == head.current_size - 1)
                    ? memory_ + size - head.array[i].start
                    : head.array[i + 1].start - head.array[i].start;
            if (available_space_after >= size) {
                head.array[i].end = head.array[i].start + size;
                void *retVal = head.array[i].start;
                pthread_rwlock_unlock(&rwlock);
                return retVal;
            }
            size_t old_block_size = head.array[i].end - head.array[i].start;
            size_t copy_size = (old_block_size > size) ? size : old_block_size;
            size_t space_before =
                (i == 0) ? head.array[i].start - memory_
                         : head.array[i].start - head.array[i - 1].end;
            void* previous_end = (i == 0) ? memory_ : head.array[i - 1].end;
            if (space_before + available_space_after >= size) {
                memmove(previous_end, head.array[i].start, copy_size);
                head.array[i].start = previous_end;
                head.array[i].end = head.array[i].start + size;
                void *retVal = head.array[i].start;
                pthread_rwlock_unlock(&rwlock);
                return retVal;
            }
            void* new_block = DA_add_mem_block__nolock__(size);
            size_t index = (head.array[i].start == block)       ? i
                           : (head.array[i + 1].start == block) ? i + 1
                                                                : i - 1;
            if (new_block) {
                memcpy(new_block, head.array[index].start, copy_size);
                // remove garbage block
                shove_left_(i);
                pthread_rwlock_unlock(&rwlock);
                return new_block;
            }
            pthread_rwlock_unlock(&rwlock);
            return NULL;
            // else, remove and add again, probably duper inefficient
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void mem_free(void* start) {
    pthread_rwlock_wrlock(&rwlock);
    for (size_t i = 0; i < head.total_size; i++) {
        if (head.array[i].start == start) {
            shove_left_(i);
            head.current_size--;
            pthread_rwlock_unlock(&rwlock);
            return;
        }
    }
    pthread_rwlock_unlock(&rwlock);
}

void mem_deinit() {
    free(head.array);
    free(memory_);
    memory_ = NULL;
    size_ = 0;
    head.array = NULL;
    head.current_size = 0;
    head.total_size = 0;
    pthread_rwlock_destroy(&rwlock);
}
