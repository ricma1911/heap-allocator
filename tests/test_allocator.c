#include "allocator.h"
#include "allocator_debug.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define CHECK_HEAP() assert(allocator_validate() == AL_DEBUG_OK)
#define NUM_OF_OPERATIONS (5000)

void test_krealloc_zero();
void test_coalescing();
void test_zero_size_allocation();
void test_allocation_failure();
void test_other_pointer();
void test_not_start_of_user_space();
void test_double_free();
void test_kmalloc();
void test_basic_krealloc();
void test_krealloc_expansion_not_enough_space();
void test_krealloc_expansion_next_block_not_free();
void test_krealloc_shrinking_next_block_not_free();
void test_krealloc_invalid_pointer();
void test_krealloc_last_block();
void test_krealloc_freed_pointer();
void test_krealloc_next_block_not_free_kmalloc();
void test_stress();



int main(){

    test_stress();
    test_krealloc_next_block_not_free_kmalloc();
    test_krealloc_freed_pointer();
    test_krealloc_last_block();
    test_krealloc_invalid_pointer();
    test_krealloc_zero();
    test_krealloc_shrinking_next_block_not_free();
    test_krealloc_expansion_next_block_not_free();
    test_krealloc_expansion_not_enough_space();
    test_basic_krealloc();
    test_kmalloc();
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
    CHECK_HEAP();
    unsigned char* new_memory_1 = (unsigned char*)kmalloc(0);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();
}


void test_coalescing(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(11);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(7);
    CHECK_HEAP();
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(16);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_3);
    CHECK_HEAP();
    kfree(new_memory_2);
    CHECK_HEAP();

}

// this kmalloc will return NULL, which indicates allocation failure
void test_allocation_failure(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();
    
    unsigned char* new_memory_1 = (unsigned char*)kmalloc(2048);
    CHECK_HEAP();
    if (new_memory_1 == NULL)
        return;
}

//&a is out of memory region and kfree will return immediately
void test_other_pointer(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    int a = 5;
    kfree(&a);
    CHECK_HEAP();
}

//new_memory_1 + 4 is not a valid pointer to user-space memory and kfree of new_memory_1 + 4 will not change allocator's state
void test_not_start_of_user_space(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(20);
    CHECK_HEAP();

    kfree(new_memory_1 + 4); //invalid pointer
    CHECK_HEAP();
    kfree(new_memory_1);
    CHECK_HEAP();
}

// a valid pointer with free = 1 will be ignored by kfree
void test_double_free(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(20);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_1); // a valid pointer
    CHECK_HEAP();
}


//another case of kmalloc is when there is not enough space 
//for metadata for other block and therefore it is impossible to make an ordinary split
//in that case allocator will give all of the remaining memory
//to the same block and there will be internal fragmentation
//this example demonstrates an edge case where exactly 24 bytes remain
//for the metadata of a new block with 0 bytes of user space
void test_kmalloc(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(11);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(32);
    CHECK_HEAP();
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(1024 - 96 - 24);
    CHECK_HEAP();

    kfree(new_memory_2);
    CHECK_HEAP();

    unsigned char* new_memory_4 = (unsigned char*)kmalloc(7);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_3);
    CHECK_HEAP();
    kfree(new_memory_4);
    CHECK_HEAP();

}

void test_basic_krealloc(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(11);
    CHECK_HEAP();

    unsigned char expected[16];
    copy_memory(expected, new_memory_1, 16);

    new_memory_1 = (unsigned char*)krealloc(new_memory_1, 19);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 16) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)kmalloc(11);
    CHECK_HEAP();
    copy_memory(expected, new_memory_1, 16);

    new_memory_1 = (unsigned char*)krealloc(new_memory_1, 7);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 8) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)kmalloc(11);
    CHECK_HEAP();
    copy_memory(expected, new_memory_1, 16);

    new_memory_1 = (unsigned char*)krealloc(new_memory_1, 977);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 16) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();

}

void test_krealloc_expansion_not_enough_space(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(16);
    CHECK_HEAP();

    kfree(new_memory_2);
    CHECK_HEAP();

    unsigned char expected[8];
    copy_memory(expected, new_memory_1, 8);

    new_memory_1 = krealloc(new_memory_1, 45);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 8) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_3);
    CHECK_HEAP();

}

void test_krealloc_expansion_next_block_not_free(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(936);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();

    unsigned char expected[8];
    copy_memory(expected, new_memory_2, 8);

    new_memory_2 = krealloc(new_memory_2, 16);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_2, expected, 8) == 1);

    kfree(new_memory_2);
    CHECK_HEAP();
    kfree(new_memory_3);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    new_memory_2 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    new_memory_3 = (unsigned char*)kmalloc(936);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();

    copy_memory(expected, new_memory_2, 8);

    new_memory_2 = krealloc(new_memory_2, 19);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_2, expected, 8) == 1);

}

void test_krealloc_shrinking_next_block_not_free(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(16);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(13);
    CHECK_HEAP();

    unsigned char expected[32];
    copy_memory(expected, new_memory_1, 16);

    new_memory_1 = krealloc(new_memory_1, 7);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 8) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_2);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)kmalloc(30);
    CHECK_HEAP();
    new_memory_2 = (unsigned char*)kmalloc(13);
    CHECK_HEAP();

    copy_memory(expected, new_memory_1, 32);

    new_memory_1 = krealloc(new_memory_1, 7);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_1, expected, 8) == 1);

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_2);
    CHECK_HEAP();

}

void test_krealloc_zero(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    new_memory_1 = krealloc(new_memory_1, 0);
    CHECK_HEAP();

}

void test_krealloc_invalid_pointer(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(8);
    CHECK_HEAP();

    int* a = krealloc(NULL, 12);
    CHECK_HEAP();
    int b = 5;
    a = krealloc(&b, 10);
    CHECK_HEAP();
    kfree(a);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();
    kfree(new_memory_2);
    CHECK_HEAP();

}

void test_krealloc_last_block(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(3);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(1024 - 56);
    CHECK_HEAP();

    unsigned char expected[968];
    copy_memory(expected, new_memory_2, 968);

    kfree(new_memory_1);
    CHECK_HEAP();

    new_memory_2 = krealloc(new_memory_2, 1000);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_2, expected, 968) == 1);

    kfree(new_memory_2);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)kmalloc(3);
    CHECK_HEAP();
    new_memory_2 = (unsigned char*)kmalloc(1024 - 56);
    CHECK_HEAP();

    copy_memory(expected, new_memory_2, 968);

    kfree(new_memory_1);
    CHECK_HEAP();

    new_memory_2 = krealloc(new_memory_2, 4);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_2, expected, 8) == 1);

    kfree(new_memory_2);
    CHECK_HEAP();

}

// krealloc on an already freed pointer must be rejected and must not change the heap
void test_krealloc_freed_pointer(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(100);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();

    new_memory_1 = (unsigned char*)krealloc(new_memory_1, 50);
    CHECK_HEAP();
    assert(new_memory_1 == NULL);
}

void test_krealloc_next_block_not_free_kmalloc(){
    unsigned char memory[1024];
    if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
        return;
    CHECK_HEAP();

    unsigned char* new_memory_1 = (unsigned char*)kmalloc(15);
    CHECK_HEAP();
    unsigned char* new_memory_2 = (unsigned char*)kmalloc(5);
    CHECK_HEAP();
    unsigned char* new_memory_3 = (unsigned char*)kmalloc(6);
    CHECK_HEAP();
    unsigned char* new_memory_4 = (unsigned char*)kmalloc(896);
    CHECK_HEAP();

    kfree(new_memory_1);
    CHECK_HEAP();

    unsigned char expected[8];
    copy_memory(expected, new_memory_3, 8);

    new_memory_3 = krealloc(new_memory_3, 16);
    CHECK_HEAP();
    assert(allocator_data_check(new_memory_3, expected, 8) == 1);

    kfree(new_memory_2);
    CHECK_HEAP();
    kfree(new_memory_3);
    CHECK_HEAP();
    kfree(new_memory_4);
    CHECK_HEAP();
}

void test_stress(){
    for (unsigned int seed = 0; seed < 5; seed++){
        unsigned char memory[1024];
        if (allocator_init(memory, sizeof(memory)) == ALLOCATOR_INVALID_SIZE)
            return;
        CHECK_HEAP();

        srand(seed);

        unsigned long num_of_successful_operations = 0;
        unsigned char* pointers[32];
        unsigned long current_len = 0;

        for (int i = 0; i < NUM_OF_OPERATIONS; i++){
            short operation_index = rand() % 2; 
            if (current_len < 32 && operation_index == 0){ //kmalloc
                unsigned long size = rand() % 50;
                unsigned char* new_memory = (unsigned char*)kmalloc(size);
                CHECK_HEAP();
                if (new_memory == NULL){
                    continue;
                }
                //printf("kmalloc size: %lu\n", size);
                pointers[current_len++] = new_memory;
                num_of_successful_operations++;

            }
            if (operation_index == 1){ //kfree
                if (current_len == 0)
                    continue;
                unsigned long pointer_index = rand() % current_len;
                kfree(pointers[pointer_index]);
                CHECK_HEAP();

                for (unsigned long j = pointer_index; j < current_len - 1;j++){
                    pointers[j] = pointers[j + 1];
                }
                //printf("kfree ptr %lu\n", pointer_index);
                num_of_successful_operations++;
                current_len--;
            }
        }
        printf("Seed %u passed!\n", seed);
        printf("Number of successful operations: %lu\n", num_of_successful_operations);
    }
}
