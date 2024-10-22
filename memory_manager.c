#define _GNU_SOURCE
#include "memory_manager.h"

typedef struct memory_block {
    void *start;
    void *end;
    struct memory_block *next;
} memory_block;

pthread_mutex_t allocation_lock;

memory_block *head;
void *memory_;
size_t size_;

/// @brief loads up the memory with memory
/// @param size
void mem_init(size_t size) {
    head = NULL;
    memory_ = malloc(size);
    size_ = size;
    pthread_mutex_init(&allocation_lock, NULL);
}

/// @brief returns pointer to memory block, NULL if no chunk of proper size
/// found
/// @param size
/// @return
void *mem_alloc(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;
    pthread_mutex_lock(&allocation_lock);

    // insertion first
    if (head == NULL || head->start - memory_ >= size) {
        memory_block *new_block = malloc(sizeof(*new_block));
        new_block->start = memory_;
        new_block->end = memory_ + size;
        new_block->next = head;
        head = new_block;
        pthread_mutex_unlock(&allocation_lock);
        return memory_;
    }

    // Insertion between blocks
    memory_block *walker = head;
    while (walker->next != NULL) {

        size_t available_space = walker->next->start - walker->end;
        if (available_space >= size) {
            memory_block *new_block = malloc(sizeof(*new_block));
            new_block->start = walker->end;
            new_block->end = walker->end + size;
            new_block->next = walker->next;
            walker->next = new_block;
            void *ret_val = walker->end;
            pthread_mutex_unlock(&allocation_lock);
            return ret_val;
        }
        walker = walker->next;
    }

    // Insertion last
    size_t available_space = memory_ + size_ - walker->end;
    if (available_space >= size) {
        memory_block *new_block = malloc(sizeof(*new_block));
        new_block->start = walker->end;
        new_block->end = walker->end + size;
        new_block->next = NULL;
        walker->next = new_block;
        void *ret_val = walker->end;
        pthread_mutex_unlock(&allocation_lock);
        return ret_val;
    }
    return NULL;
}
void *mem_alloc__nolock__(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;

    // insertion first
    if (head == NULL || head->start - memory_ >= size) {
        memory_block *new_block = malloc(sizeof(*new_block));
        new_block->start = memory_;
        new_block->end = memory_ + size;
        new_block->next = head;
        head = new_block;
        return memory_;
    }

    // Insertion between blocks
    memory_block *walker = head;
    while (walker->next != NULL) {
        size_t available_space = walker->next->start - walker->end;
        if (available_space >= size) {
            memory_block *new_block = malloc(sizeof(*new_block));
            new_block->start = walker->end;
            new_block->end = walker->end + size;
            new_block->next = walker->next;
            walker->next = new_block;
            void *ret_val = walker->end;
            return ret_val;
        }
    }

    // Insertion last
    size_t available_space = memory_ + size_ - walker->end;
    if (available_space >= size) {
        memory_block *new_block = malloc(sizeof(*new_block));
        new_block->start = walker->end;
        new_block->end = walker->end + size;
        new_block->next = NULL;
        walker->next = new_block;
        void *ret_val = walker->end;
        return ret_val;
    }
    return NULL;
}

/// @brief Frees the memory block preventing memory leaks
/// @param block
void mem_free(void *block) {
    pthread_mutex_lock(&allocation_lock);
    if (head->start == block) {
        memory_block *temp = head;
        head = head->next;
        free(temp);
        pthread_mutex_unlock(&allocation_lock);
        return;
    }
    memory_block *walker = head;
    while (walker->next != NULL) {
        if (walker->next->start == block) {
            memory_block *temp = walker->next;
            walker->next = temp->next;
            free(temp);
            pthread_mutex_unlock(&allocation_lock);
            return;
        }
        walker = walker->next;
    }
    pthread_mutex_unlock(&allocation_lock);
    return;
}
/// @brief changes the size of the block, if possible without moving it, returns
/// NULL if failed
/// @param block
/// @param size
/// @return
void *mem_resize(void *block, size_t size) {
    if (size > size_) return NULL;
    if (!block) return mem_alloc(size);
    pthread_mutex_lock(&allocation_lock);

    // Save some data about the deleted node incase we need to replace it
    memory_block *before_node = head;
    if(head->start == block) before_node = NULL;
    else while(before_node->next != NULL && before_node->next->start != block) before_node = before_node->next;

    // Node doesn't exist
    if( before_node->next == NULL) {
        pthread_mutex_unlock(&allocation_lock);
        return NULL;
    }
    memory_block delete_copy = (before_node) ? *(before_node->next) : *head;
    before_node->next = before_node->next->next;
    free(before_node->next);

    if (size == 0){
        pthread_mutex_unlock(&allocation_lock);
        return NULL;
    }
    void *newblock = mem_alloc__nolock__(size);
    // if no space for resize replace deleted block
    if(!newblock){
        memory_block *new_old_block = malloc(sizeof(*new_old_block));
        *new_old_block = delete_copy;
        before_node->next = new_old_block;
        pthread_mutex_unlock(&allocation_lock);
        return NULL;
    }
    pthread_mutex_unlock(&allocation_lock);
    return newblock;

}

/// @brief gives back the memory used by the memory manager
void mem_deinit() {
    memory_block *walker = head;
    while(walker != NULL){
        memory_block *temp = walker;
        walker = walker->next;
        free(temp);
    }
    free(memory_);
    size_ = 0;
    head = NULL;
    pthread_mutex_destroy(&allocation_lock);
}
