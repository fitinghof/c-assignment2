#define _GNU_SOURCE
#include "dynamic_array.h"

#include <pthread.h>

/*
    Merge memory manager and dynamic array?

    Should probably implement locks for each memoryblock or array index or
   whatever and have thread's trying to loop over those be locked until the
   thread infron is finished threads should ofcourse also never be allowed to
   stop on the same index when looping might be mega complicated to implement
   but it's a consideration. It might allow for function to have a "soft" write
   lock, allowing changes to be made in that index, like perhaps setting a
   memoryblock as used

    A split function would probably be good, this function would require
   complete array write/read protection but could probably be made fairly fast.

    Dynamic array? :)
        + low memory overhead
        + fast to iterate over
        - splitting would likely require locking the entire thing ( If I even go
   the splitting route )


    linked list? :(
        + splitting would be trivial
        + locking would probably be quit simple
        + since locking is simple concurrency would likely be high
        - linked list is slow
        - lots of messing with freeing nodes
        - takes alot of memory

*/

pthread_rwlock_t rwlock;

void shove_left_(dynamic_array_head* head, size_t index) {}

void shove_right_(dynamic_array_head* head, size_t index) {}

void resize_(dynamic_array_head* head, size_t new_size) {}

void add_last_(dynamic_array_head* head, memory_block block) {
    if (head->current_size == head->total_size)
        resize_(head, head->current_size * 2);
    head->array[head->current_size++] = block;
    return;
}

void DA_init(dynamic_array_head* head, size_t init_size) {
    head->array = calloc(init_size, sizeof(*(head->array)));
    head->current_size = 0;
    head->total_size = init_size;
    pthread_rwlock_init(&rwlock, NULL);
}

void* DA_add_mem_block(dynamic_array_head* head, size_t size) {
    pthread_rwlock_wrlock(&rwlock);
    memory_block *walker = DA_get_first__nolock__(head);
    uint32_t pointer_ofset = 0;
    while(walker != NULL){
        while(!walker->free) {
            walker = DA_get_next__nolock__(head, walker);
        }
        uint32_t total_free = 0;
        memory_block *free_block_start = walker;
        while(walker->free) {
            total_free += walker->size;
            walker = DA_get_next__nolock__(head, walker);
        }
        if(total_free >= size){

        }
    }

    pthread_rwlock_unlock(&rwlock);
    return NULL;
}

void DA_remove(dynamic_array_head* head, memory_block* block) {

}

memory_block* DA_get_next__nolock__(dynamic_array_head* head, memory_block* block) {
    block++;
    while (block < head->array + head->total_size) {
        if(block->exists) {
            pthread_rwlock_unlock(&rwlock);
            return block;
        }
        block++;
    }
    return NULL;
}

memory_block* DA_get_first__nolock__(dynamic_array_head* head) {
    if (head->array[0].exists) return head->array;
    return DA_get_next__nolock__(head, head->array);
}

void DA_deinit(dynamic_array_head* head) {
    free(head->array);
    pthread_rwlock_destroy(&rwlock);
}
