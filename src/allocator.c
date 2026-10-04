#include "allocator.h"

typedef struct block{
    unsigned long size;
    unsigned long free;
    struct block* next;
}block;

block* first_block;

void allocator_init(void* region, unsigned long size){
    first_block = region;
    first_block->free = 1;
    first_block->size = size;
    first_block->next = NULL;
}

void* kmalloc(unsigned long size){
    block* current_block = first_block;
    while(1){
        if (current_block == NULL) {
            return 0;
        }
        if (current_block->free == 1 && size <= current_block->size){
            unsigned long current_size = current_block->size;
            unsigned long current_free = current_block->free;
            block* current_next = current_block->next;

            block* new_block = current_block;
            current_block = (block*)((unsigned char*)current_block + size);

            current_block->size = current_size - size;
            current_block->free = current_free;
            current_block->next = current_next;

            new_block->size = size;
            new_block->free = 0;
            new_block->next = current_block;

            return new_block;
        }
        current_block = current_block->next;
    }
}