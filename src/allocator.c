#include "allocator.h"

typedef struct {
    unsigned long size;
    unsigned long free;
}block;

void allocator_init(void* region, unsigned long size){
    block* first_block = region;
    first_block->free = 1;
    first_block->size = size;
}