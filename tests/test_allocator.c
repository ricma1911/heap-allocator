#include "allocator.h"


void test_coalescing();
void test_zero_size_allocation();
void test_allocation_failure();
void test_other_pointer();
void test_not_start_of_user_space();
void test_double_free();

int main(){

    test_double_free();
    test_not_start_of_user_space();
    test_other_pointer();
    test_allocation_failure();
    test_zero_size_allocation();
    test_coalescing();

    return 0;
}


//kmalloc(0) will create block with 24 bytes of metadata and 0 user-space bytes
void test_zero_size_allocation(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    unsigned char* new_memory_1 = (unsigned char*)kmalloc(0);
    kfree(new_memory_1);
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

// this kmalloc will return NULL, which indicates allocation failure
void test_allocation_failure(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    
    unsigned char* new_memory_1 = (unsigned char*)kmalloc(2048);
    if (new_memory_1 == NULL)
        return;
}

//&a is out of memory region and kfree will return immediately
void test_other_pointer(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;

    int a = 5;
    kfree(&a);
}

//new_memory_1 + 4 is not a valid pointer to user-space memory and kfree of new_memory_1 + 4 will not change allocator's state
void test_not_start_of_user_space(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(20);
    kfree(new_memory_1 + 4); //invalid pointer
    kfree(new_memory_1);
}

// a valid pointer with free = 1 will be ignored by kfree
void test_double_free(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(20);
    kfree(new_memory_1);
    kfree(new_memory_1); // a valid pointer
}

