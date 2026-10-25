CFLAGS = -std=c17 -Wall -Wextra

allocator.o: src/allocator.c
	gcc $(CFLAGS) -ffreestanding -Iinclude -g -c src/allocator.c

test: test_allocator.o allocator_test_mode.o
	gcc $(CFLAGS) test_allocator.o allocator_test_mode.o -o test_allocator

test_allocator.o: tests/test_allocator.c
	gcc $(CFLAGS) -Iinclude -g -c tests/test_allocator.c

allocator_test_mode.o: src/allocator.c
	gcc $(CFLAGS) -ffreestanding -Iinclude -DALLOCATOR_DEBUG -g -c src/allocator.c -o allocator_test_mode.o

clean:
	rm -rf *.o test_allocator