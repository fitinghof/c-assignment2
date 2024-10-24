#define _GNU_SOURCE
#include "memory_manager.h"

typedef struct Node {
    void *start;
    void *end;
    struct Node *next;
} Node;

Node *node_factory(void *start, void *end, Node *next) {
    Node *new_block = malloc(sizeof(*new_block));
    new_block->start = start;
    new_block->end = end;
    new_block->next = next;
    return new_block;
}

pthread_mutex_t allocation_lock;

Node *free_head;
Node *busy_head;
void *memory_;
size_t size_;

/// @brief Initiates the memory mannager with @p size bytes of memory
/// @param size bytes that will be available in the memory manager
void mem_init(size_t size) {
    free_head = NULL;
    busy_head = NULL;
    memory_ = malloc(size);
    size_ = size;
    pthread_mutex_init(&allocation_lock, NULL);
}

/// @brief Allocates @p size bytes of memory
/// @param @p size number of bytes that will be allocated
/// @return pointer to the allocated memory
void *mem_alloc(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;
    pthread_mutex_lock(&allocation_lock);

    Node* prev_walker = NULL;
    Node* walker = free_head;
    while(walker != NULL) {
        size_t space = walker->end - walker->start;
        if(space > size){
            void *ret_val = walker->start;
            busy_head = node_factory(ret_val, ret_val + size, busy_head);
            walker->start = ret_val + size;
            pthread_mutex_unlock(&allocation_lock);
            return ret_val;
        }
        else if (space == size){
            void *ret_val = walker->start;
            busy_head = node_factory(ret_val, ret_val + size, busy_head);
            prev_walker->next = walker->next;
            free(walker);
            pthread_mutex_unlock(&allocation_lock);
            return ret_val;
        }
        prev_walker = walker;
        walker = walker->next;
    }

    pthread_mutex_unlock(&allocation_lock);
    return NULL;
}

/// @brief A nolock copy of the mem_alloc function that can be used for alloc
/// when a lock has already been aquired
/// @param size bytes that should be allocated
/// @return pointer to allocated memory
void *mem_alloc__nolock__(size_t size) {
    if (size > size_) return NULL;
    if (size == 0) return memory_;

    // insertion first
    if (free_head == NULL || free_head->start - memory_ >= size) {
        Node *new_block = node_factory(memory_, memory_ + size, free_head);
        free_head = new_block;
        return memory_;
    }

    // Insertion between blocks or last
    Node *walker = free_head;
    while (walker != NULL) {
        size_t available_space = (walker->next)
                                     ? walker->next->start - walker->end
                                     : memory_ + size_ - walker->end;
        if (available_space >= size) {
            Node *new_block = node_factory(
                walker->end, walker->end + size, walker->next);
            walker->next = new_block;
            walker->next = new_block;
            void *ret_val = walker->end;
            return ret_val;
        }
        walker = walker->next;
    }
    return NULL;
}



/// @brief Frees @p block preventing memory leaks
/// @param block
void mem_free(void *block) {
    pthread_mutex_lock(&allocation_lock);
    // No nodes
    if(block == NULL || !busy_head) return;

    // find and free head;
    Node* free_block;
    if(busy_head->start == block){
        free_block = busy_head;
        busy_head = busy_head->next;
    }

    // find and free head;
    Node* walker = busy_head;
    while ( walker->next != NULL && walker->next->start != block )
        walker = walker->next;
    if( !walker ){
        return;
    }
    free_block = walker->next;
    walker->next = free_block->next;

    // Incase the block should be the first in the free list
    if(block <= free_head->start) {
        if(free_block->end == free_head->start){
            free_head->start = block;
            free(free_block);

            pthread_mutex_unlock(&allocation_lock);
            return;
        }
        else {
            free_block->next = free_head;
            free_head = free_block;

            pthread_mutex_unlock(&allocation_lock);
            return;
        }
    }

    // Find the right place for our free space
    Node* prev_walker = free_head;
    walker = free_head->next;
    while(walker != NULL && walker->start <= free_block->end) {
        // Merge previous free
        if ( walker->end == block ){
            // Might also be able to merge next free
            if(walker->next != NULL && walker->next->start == free_block->end)
                walker->end = walker->next->end;
            else
                walker->end = free_block->end;
            pthread_mutex_unlock(&allocation_lock);
            free(free_block);
            return;
        }
        // Merge next free block
        else if (walker->start == block){
            prev_walker->end = free_block->end;
            free(free_block);
            pthread_mutex_unlock(&allocation_lock);
            return;
        }
        prev_walker = walker;
        walker = walker->next;
    }
    prev_walker->next = free_block;
    free_block->next = walker;
    pthread_mutex_unlock(&allocation_lock);
    return;
}
/// @brief Changes the size of the allocated block, return NULL if failed
/// @param block pointer to your allocated memory, if NULL allocates new memory
/// of @p size
/// @param size The new size of your allocated memory, if 0 mem_free is called
/// for @p block
/// @return
void *mem_resize(void *block, size_t size) {
    if (!block) return mem_alloc(size);
    if (!size) {
        mem_free(block);
        return;
    }
    pthread_mutex_lock(&allocation_lock);
    Node *block_node = busy_head;
    while(block_node != NULL && block_node->start != block) {
        block_node = block_node->next;
    }
    if(!block_node){
        pthread_mutex_unlock(&allocation_lock);
        return NULL;
    }

    
    pthread_mutex_unlock(&allocation_lock);
}

/// @brief gives back the memory used by the memory manager
void mem_deinit() {
    Node *walker = free_head;
    while (walker != NULL) {
        Node *temp = walker;
        walker = walker->next;
        free(temp);
    }
    free(memory_);
    size_ = 0;
    free_head = NULL;
    pthread_mutex_destroy(&allocation_lock);
}
