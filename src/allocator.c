#include "allocator.h"


//normally this structure takes 24 bytes
typedef struct block{
    unsigned long size; //this is user-free space
    unsigned long free;
    struct block* next;
}block;

block* first_block;
unsigned char* heap_start;
unsigned char* heap_end;


allocator_status allocator_init(void* region, unsigned long size){
    if (size <= sizeof(block)){
        return ALLOCATOR_INVALID_SIZE;
    }
    first_block = region;
    first_block->free = 1;
    first_block->size = size - sizeof(block);
    first_block->next = NULL;

    heap_start = region;
    heap_end = heap_start + size;

    return ALLOCATOR_OK;
}

void* kmalloc(unsigned long size){
    while(size % 8 != 0)
        size++;
    block* current_block = first_block;
    while(1){
        if (current_block == NULL) {
            return NULL;
        }
        if (current_block->free == 1 && size <= current_block->size){

            if (current_block->size - size < sizeof(block)){
                current_block->free = 0;
                return (unsigned char*) current_block + sizeof(block);
            }
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
    unsigned char* p = ptr;
    if (p < heap_start || p >= heap_end) { //memory is not in allocator's ownership
        return;
    }

    block* free_block = (block*)((unsigned char*) ptr - sizeof(block));

    short found = 0;
    block* current_block = first_block;
    if (current_block == free_block)
        found = 1;
    while (current_block->next != NULL){
        current_block = current_block->next;
        if (found == 1 || current_block == free_block){
            found = 1;
            break;
        }
    }

    if (!found)
        return;

    if (free_block->free == 1){
        return;
    }
    
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

void* krealloc(void* ptr, unsigned long size){
    while (size % 8 != 0)
        size++;
    
    block* current_block = (block*) ((unsigned char*)(ptr - sizeof(block)));
    block* next_block = current_block->next;

    if (size == current_block->size){
        return (unsigned char*) current_block + sizeof(block);
    }

    if (size > current_block->size) { // block expansion

        unsigned long byte_difference = size - current_block->size;

        if (next_block->free == 1){
            if (next_block->size >= byte_difference){//there is enough space for second block metadata
                current_block->size = size;
                next_block->size -= byte_difference;

                block* new_next_block = (block*)((unsigned char*) next_block + byte_difference);
                current_block->next = new_next_block;

                new_next_block->next = next_block->next;
                new_next_block->free = next_block->free;
                new_next_block->size = next_block->size;

            } else { // not enough space for second block metadata

                if (byte_difference <= next_block->size + sizeof(block)){ // there is enough space
                    current_block->size += next_block->size + sizeof(block);
                    current_block->next = next_block->next;

                } else { // not enough space

                    unsigned char* new_data = (unsigned char*)kmalloc(size);
                    if (new_data == NULL) { //kmalloc failed
                        return NULL; //means krealloc failed
                    }

                    block* new_created_block = (block*)((unsigned char*) new_data - sizeof(block));

                    //copy of old data
                    unsigned char* old_data = (unsigned char*) current_block + sizeof(block);

                    for (int i = 0; i < current_block->size;i++){
                        *new_data = *old_data;
                        old_data += 1;
                        new_data += 1;
                    }

                    //free of the old block
                    kfree((unsigned char*) current_block + sizeof(block));
                    return (unsigned char*)new_created_block + sizeof(block);
                }
                
            }
        } else {

        }

        return (unsigned char*) current_block + sizeof(block);

    } else { // block shrinking

        unsigned long byte_difference = current_block->size - size;
        if (next_block->free == 1) {
            current_block->size = size;
            next_block->size += byte_difference;

            block* new_next_block = (block*) ((unsigned char*) next_block - byte_difference);
            current_block->next = new_next_block;

            new_next_block->size = next_block->size;
            new_next_block->free = next_block->free;
            new_next_block->next = next_block->next;
        }

        return (unsigned char*) current_block + sizeof(block);
    }
}

