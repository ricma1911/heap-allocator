#include "allocator.h"


void test_coalescing();

int main(){

    test_coalescing();

    return 0;
}

void test_coalescing(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(11);
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(7);
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(16);

    kfree(new_memory_1);
    kfree(new_memory_3);
    kfree(new_memory_2);

}