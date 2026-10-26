#include "allocator.h"

#ifdef ALLOCATOR_DEBUG
#include "allocator_debug.h"
#endif

//normally this structure takes 24 bytes
typedef struct block{
    unsigned long size; //this is user-free space
    unsigned long free;
    struct block* next;
}block;

static block* first_block;
#ifdef ALLOCATOR_DEBUG
static unsigned long region_size = 0;
#endif

allocator_status allocator_init(void* region, unsigned long size){
    if (size <= sizeof(block)){
        return ALLOCATOR_INVALID_SIZE;
    }
    first_block = region;
    first_block->free = 1;
    first_block->size = size - sizeof(block);
    first_block->next = NULL;

    #ifdef ALLOCATOR_DEBUG
    region_size = size;
    #endif

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
    if (ptr == NULL)
        return;
    
    short found = 0;
    block* current_block = first_block;
    if ((unsigned char*)current_block + sizeof(block) == ptr)
        found = 1;
    while (current_block->next != NULL){
        current_block = current_block->next;
        if (found == 1 || (unsigned char*)current_block + sizeof(block) == ptr){
            found = 1;
            break;
        }
    }

    if (!found)
        return;

    block* free_block = (block*)((unsigned char*) ptr - sizeof(block));

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
    if (ptr == NULL){
        return NULL;
    }

    short found = 0;
    block* current_block = first_block;
    if ((unsigned char*)current_block + sizeof(block) == ptr)
        found = 1;
    while (current_block->next != NULL){
        current_block = current_block->next;
        if (found == 1 || (unsigned char*)current_block + sizeof(block) == ptr){
            found = 1;
            break;
        }
    }

    if (!found)
        return NULL;

    if (size == 0){
        kfree(ptr);
        return NULL;
    }

    while (size % 8 != 0)
        size++;
    
    current_block = (block*) ((unsigned char*) ptr - sizeof(block));
    block* next_block = current_block->next;

    if (size == current_block->size){
        return (unsigned char*) current_block + sizeof(block);
    }

    if (size > current_block->size) { // block expansion

        unsigned long byte_difference = size - current_block->size;

        if (next_block != NULL && next_block->free == 1){
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

                    for (unsigned long i = 0; i < current_block->size;i++){
                        *new_data = *old_data;
                        old_data += 1;
                        new_data += 1;
                    }

                    //free of the old block
                    kfree((unsigned char*) current_block + sizeof(block));
                    return (unsigned char*)new_created_block + sizeof(block);
                }
                
            }
        } else { //next block is not free or does not exist

                unsigned char* new_data = (unsigned char*)kmalloc(size);
                if (new_data == NULL) { //kmalloc failed, in this case we will try with the previous block
                    if (first_block == current_block) { //no previous
                        return NULL; //fail
                    }
                    
                    block* previous_block = first_block;
                    while (previous_block->next != current_block){
                        previous_block = previous_block->next;
                    }

                    if (previous_block->free == 0){ // not free
                        return NULL; //fail
                    }

                    //implementation for previous block
                    if (byte_difference <= previous_block->size){ // there is enough space for previous block metadata
                        previous_block->size -= byte_difference;
                        block* new_block = current_block;

                        unsigned long old_size = current_block->size;

                        new_block = (block*) ((unsigned char*) current_block - byte_difference);
                        new_block->size = size;
                        new_block->free = current_block->free;
                        new_block->next = current_block->next;

                        previous_block->next = new_block;

                        unsigned char* new_data = (unsigned char*) new_block + sizeof(block);
                        unsigned char* old_data = (unsigned char*) current_block + sizeof(block);
                        
                        for (unsigned long i = 0; i < old_size;i++){
                            *new_data = *old_data;
                            old_data += 1;
                            new_data += 1;
                        }

                        return (unsigned char*) new_block + sizeof(block);
                    }

                    if (byte_difference <= previous_block->size + sizeof(block)) { // not enough space for previous block metadata, but enough space for reallocation
                        previous_block->size = current_block->size + previous_block->size + sizeof(block);
                        previous_block->free = current_block->free;
                        previous_block->next = current_block->next;

                        unsigned long old_size = current_block->size;

                        unsigned char* new_data = (unsigned char*) previous_block + sizeof(block);
                        unsigned char* old_data = (unsigned char*) current_block + sizeof(block);
                        
                        for (unsigned long i = 0; i < old_size;i++){
                            *new_data = *old_data;
                            old_data += 1;
                            new_data += 1;
                        }

                        return (unsigned char*)previous_block + sizeof(block);
                    }

                    return NULL; //not enough space
                }

                block* new_created_block = (block*)((unsigned char*) new_data - sizeof(block));

                //copy of old data
                unsigned char* old_data = (unsigned char*) current_block + sizeof(block);

                for (unsigned long i = 0; i < current_block->size;i++){
                    *new_data = *old_data;
                    old_data += 1;
                    new_data += 1;
                }

                //free of the old block
                kfree((unsigned char*) current_block + sizeof(block));
                return (unsigned char*)new_created_block + sizeof(block);

        }

        return (unsigned char*) current_block + sizeof(block);

    } else { // block shrinking

        unsigned long byte_difference = current_block->size - size;
        if (next_block != NULL && next_block->free == 1) {
            current_block->size = size;
            next_block->size += byte_difference;

            block* new_next_block = (block*) ((unsigned char*) next_block - byte_difference);
            current_block->next = new_next_block;

            new_next_block->size = next_block->size;
            new_next_block->free = next_block->free;
            new_next_block->next = next_block->next;
        }else {
            if (byte_difference < sizeof(block)){ /// not enough space for another block metadata
                return (unsigned char*) current_block + sizeof(block);
            }   

            current_block->size = size;
            block* new_next_block = (block*) ((unsigned char*) current_block + sizeof(block) + size);

            new_next_block->size = byte_difference - sizeof(block);
            new_next_block->free = 1;
            new_next_block->next = current_block->next;

            current_block->next = new_next_block;

            return (unsigned char*) current_block + sizeof(block);
        }

        return (unsigned char*) current_block + sizeof(block);
    }
}

#ifdef ALLOCATOR_DEBUG
debug_status allocator_validate(){
    block* current_block = first_block;
    unsigned long current_size = 0;

    unsigned long previous_free = 0;
    short previous_free_ind = 0; //indicator whether previous free has info of a previous block

    while(current_block != NULL){
        if (current_block->free != 0 && current_block->free != 1){
            return AL_DEBUG_INVALID_FREE_VALUE;
        }

        if (previous_free_ind == 1){
            if (current_block->free == 1 && previous_free == 1){
                return AL_DEBUG_TWO_FREE_BLOCKS_SEQUENCE;
            }
        }

        if (current_block->size % 8 != 0){
            return AL_DEBUG_INVALID_SIZE;
        }

        if (current_block->next != NULL && current_block->next != (block*)((unsigned char*) current_block + sizeof(block) + current_block->size)){
            return AL_DEBUG_INVALID_NEXT;
        }

        current_size += sizeof(block) + current_block->size;
        previous_free = current_block->free;
        previous_free_ind = 1;
        current_block = current_block->next;
    }

    if (current_size != region_size) {
        return AL_DEBUG_INVALID_TOTAL_SIZE;
    }

    return AL_DEBUG_OK;
}
#endif