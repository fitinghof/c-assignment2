#include "memory_manager.h"

bool* occupancy_array_;
bool* block_end_points_;
void* start_;
size_t size_;

void mem_init(size_t size){
    start_ = malloc(size);
    size_ = size;
    occupancy_array_ = malloc(sizeof(bool[size]));
    block_end_points_ = malloc(sizeof(bool[size]));
}

void* mem_alloc(size_t size) {
    if(size == 0) return NULL;

    size_t current_empty_blocks = 0;

    for (size_t i = 0; i < size_; i++)
    {
        current_empty_blocks = (occupancy_array_[i] == 0) ? current_empty_blocks + 1 : 0;

        if(current_empty_blocks == size){
            block_end_points_[i] = 1;
            for (size_t j = 0; j < size; j++)
            {
                occupancy_array_[i - j] = 1;
            }
            return start_ + (i + 1 - size);
        }
    }
    return NULL;
}

void mem_free(void* block){
    if(block == NULL) return;

    size_t block_index = block - start_;
    while(block_end_points_[block_index] != 1)
    {
        occupancy_array_[block_index] = 0;
        block_index++;
    }
    occupancy_array_[block_index] = 0;
    block_end_points_[block_index] = 0;
}

void* mem_resize(void* block, size_t size){
    size_t index = block - start_;
    size_t available_space = 0;
    while(block_end_points_[index] == 0){
        available_space++;
        index++;
    }
    size_t current_end_point = index;
    index++;
    available_space++;
    size_t old_size = available_space;
    while(block_end_points_[index] != 1 && available_space < size)
    {
        available_space++;
        index++;
    }
    if(available_space == size){
        block_end_points_[current_end_point] = 0;
        block_end_points_[index] = 1;
        return block;
    }
    else{
        void* new_block = mem_alloc(size);
        index = 0;
        int new_size = (size < old_size) ? size : old_size;
        memcpy(new_block, block, new_size);
        mem_free(block);
    }
}

void mem_deinit(){
    free(start_);
}