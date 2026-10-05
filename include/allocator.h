#define NULL 0

typedef enum {
    ALLOCATOR_OK,
    ALLOCATOR_INVALID_SIZE
} allocator_status;

allocator_status allocator_init(void* region, unsigned long size);

void* kmalloc(unsigned long size);

void kfree(void* ptr);