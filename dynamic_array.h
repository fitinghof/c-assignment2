#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct memory_block {
    void* start;
    void* end;
} memory_block;

typedef struct dynamic_array_head {
    memory_block* array;
    size_t total_size;
    size_t current_size;
} dynamic_array_head;

void DA_init(dynamic_array_head *head, size_t init_size);

void DA_add(dynamic_array_head *head, memory_block block);

void DA_remove(dynamic_array_head* head, memory_block* block);

memory_block *DA_find(dynamic_array_head *head, void *start);

memory_block *DA_get_next(dynamic_array_head *head, memory_block *block);

memory_block *DA_get_first(dynamic_array_head *head);

void DA_deinit(dynamic_array_head *head);


#endif