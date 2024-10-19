#include "memory_manager.h"

pthread_mutex_t allocation_lock;

void *memory_;
dynamic_array_head blocks_;
size_t size_;

/// @brief loads up the memory with memory
/// @param size
void mem_init(size_t size) {

}

/// @brief returns pointer to memory block, NULL if no chunk of proper size
/// found
/// @param size
/// @return
void* mem_alloc(size_t size){

}

/// @brief Frees the memory block preventing memory leaks
/// @param block
void mem_free(void *block) {

}
/// @brief changes the size of the block, if possible without moving it, returns
/// NULL if failed
/// @param block
/// @param size
/// @return
void *mem_resize(void *block, size_t size) {

}


/// @brief gives back the memory used by the memory manager
void mem_deinit() {

}