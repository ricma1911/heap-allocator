#include "allocator.h"

int main(){

    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return 0;

    unsigned char* new_memory = (unsigned char*)kmalloc(11);
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(16);




    return 0;
}