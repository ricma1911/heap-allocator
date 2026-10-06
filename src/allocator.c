#include "allocator.h"


//normally this structure takes 24 bytes
typedef struct block{
    unsigned long size; //this is user-free space
    unsigned long free;
    struct block* next;
}block;

block* first_block;

allocator_status allocator_init(void* region, unsigned long size){
    if (size <= sizeof(block)){
        return ALLOCATOR_INVALID_SIZE;
    }
    first_block = region;
    first_block->free = 1;
    first_block->size = size - sizeof(block);
    first_block->next = NULL;
    return ALLOCATOR_OK;
}

void* kmalloc(unsigned long size){
    while(size % 8 != 0)
        size++;
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
            current_block = (block*)((unsigned char*)current_block + size + sizeof(block));

            current_block->size = current_size - size - sizeof(block);
            current_block->free = current_free;
            current_block->next = current_next;

            new_block->size = size;
            new_block->free = 0;
            new_block->next = current_block;

            return (unsigned char*)new_block + sizeof(block);
        }
        current_block = current_block->next;
    }
}

void kfree(void* ptr){
    block* free_block = (block*)((unsigned char*) ptr - sizeof(block));
    free_block->free = 1;
    if (free_block->next != NULL && free_block->next->free == 1){
        free_block->size = free_block->size + free_block->next->size + sizeof(block);
        free_block->next = free_block->next->next;
    }

    if (free_block != first_block) { //if it is a first block there is no previous
        block* current_block = first_block;
        while (current_block->next != free_block)
            current_block = current_block->next;
        if (current_block->free == 1){
            current_block->size = current_block->size + free_block->size + sizeof(block);
            current_block->next = current_block->next->next;
        }

    }
    
}