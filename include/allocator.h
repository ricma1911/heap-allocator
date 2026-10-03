#define NULL 0

void allocator_init(void* region, unsigned long size);

void* kmalloc(unsigned long size);

void kfree(void* ptr);