#include "allocator.h"

int main(){

    unsigned char memory[1024];
    allocator_init(memory, sizeof(memory));

    unsigned char* new_memory = (unsigned char*)kmalloc(24);
    return 0;
}