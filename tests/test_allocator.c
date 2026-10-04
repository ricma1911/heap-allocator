#include "allocator.h"

int main(){

    unsigned char memory[1024];
    allocator_init(memory, sizeof(memory));
    return 0;
}