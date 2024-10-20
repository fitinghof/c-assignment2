#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H
#define __USE_UNIX98 1

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    uint32_t size;
    bool free;
    bool exists;
} memory_block;

typedef struct dynamic_array_head {
    memory_block *array;
    size_t total_size;
    size_t current_size;
} dynamic_array_head;

void DA_init(dynamic_array_head *head, size_t init_size);

void *DA_add_mem_block(dynamic_array_head *head, size_t size);

void DA_remove(dynamic_array_head *head, memory_block *block);

memory_block *DA_find(dynamic_array_head *head, void *start);

memory_block *DA_get_next(dynamic_array_head *head, memory_block *block);

memory_block *DA_get_first__nolock__(dynamic_array_head *head);

void DA_deinit(dynamic_array_head *head);

#endif