#define _GNU_SOURCE
#include "dynamic_array.h"
#include <pthread.h>
#include <pthread.h>


pthread_rwlock_t rwlock;

void pack_memory(dynamic_array_head* head) {

    size_t current_index = 0;
    for (size_t i = 0; i < head->total_size; i++) {
        if (head->array[i].start && i != current_index) {
            head->array[current_index++] = head->array[i];
            if (current_index == (head->current_size - 1)) return;
        }
    }
}

bool resize(dynamic_array_head* head, size_t new_size) {
    if (head->total_size < new_size){
        void *temp = realloc(head->array, new_size * sizeof(memory_block));
        if(!temp) return false;
        head->array = temp;
        for (size_t i = head->current_size; i < new_size; i++)
            head->array[i] = (memory_block){NULL, NULL};
    }
    else {
        if (head->current_size < new_size) return false;
        pack_memory(head);
        void *temp = realloc(head->array, new_size * sizeof(memory_block));
        if(!temp) return false;
        head->array = temp;
    }
    head->total_size = new_size;
    return true;
}

void DA_init(dynamic_array_head* head, size_t init_size) {
    head->array = malloc(init_size * sizeof(memory_block));
    head->current_size = 0;
    head->total_size = init_size;
    for (size_t i = 0; i < head->total_size; i++) {
        head->array[i] = (memory_block){NULL, NULL};
    }
    pthread_rwlock_init(&rwlock, NULL);
}

void shove_left_(dynamic_array_head* head, size_t index) {
    memory_block temp = {NULL, NULL};
    while (head->array[index].start != NULL && index != 0) {
        temp = head->array[index - 1];
        head->array[index - 1] = head->array[index];
        index--;
    }
    head->array[index] = temp;
}

void shove_right_(dynamic_array_head* head, size_t index) {
    memory_block temp;
    while (head->array[index].start != NULL && index != head->total_size - 2) {
        temp = head->array[index + 1];
        head->array[index + 1] = head->array[index];
        index++;
    }
    head->array[index] = temp;
}

void DA_add(dynamic_array_head* head, memory_block block) {
    pthread_rwlock_wrlock(&rwlock);
    if (head->current_size == head->total_size) {
        resize(head, head->total_size * 2);
    }
    if (head->current_size == 0) {
        head->array[(head->current_size)++] = block;

        pthread_rwlock_unlock(&rwlock);
        return;
    }
    bool has_empty_before = false;
    size_t blocks_left = head->current_size;
    for (size_t i = 0; i < head->total_size; i++) {
        if (head->array[i].start == NULL && blocks_left)
            has_empty_before = true;
        else if (block.start < head->array[i].start || blocks_left == 0) {
            head->current_size++;
            if (has_empty_before) {
                shove_left_(head, i - 1);
                head->array[i - 1] = block;

                pthread_rwlock_unlock(&rwlock);
                return;
            } else {
                shove_right_(head, i);
                head->array[i] = block;

                pthread_rwlock_unlock(&rwlock);
                return;
            }
        }
        blocks_left--;
    }
    pthread_rwlock_unlock(&rwlock);
}

void DA_remove(dynamic_array_head* head, memory_block* block) {
    if(!block) return;
    pthread_rwlock_wrlock(&rwlock);
    head->current_size--;
    block->start = NULL;
    block->end = NULL;
    pthread_rwlock_unlock(&rwlock);
}

memory_block* DA_find(dynamic_array_head* head, void* start) {
    pthread_rwlock_rdlock(&rwlock);

    for (size_t i = 0; i < head->total_size; i++) {
        if (head->array[i].start == start)
        {
            pthread_rwlock_unlock(&rwlock);
            return &(head->array[i]);
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return NULL;

}

memory_block* DA_get_next(dynamic_array_head* head, memory_block* block) {
    pthread_rwlock_rdlock(&rwlock);
    block++;
    while (block < (head->array + head->total_size)) {
        if (block->start != NULL) {
            pthread_rwlock_unlock(&rwlock);
            return block;
        }
        block++;
    }
    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

memory_block *DA_get_first(dynamic_array_head *head){
    if(head->current_size == 0) return NULL;
    pthread_rwlock_rdlock(&rwlock);
    memory_block *block = &head->array[0];
    while(block->start == NULL) block++;
    pthread_rwlock_unlock(&rwlock);
    return block;
}


void DA_deinit(dynamic_array_head *head){
    free(head->array);
    pthread_rwlock_destroy(&rwlock);
}
