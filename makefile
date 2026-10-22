test_allocator: test_allocator.o allocator.o
	gcc -Wall -Wextra test_allocator.o allocator.o -o test_allocator

test_allocator.o: tests/test_allocator.c
	gcc -Wall -Wextra -Iinclude -g -c tests/test_allocator.c

allocator.o: src/allocator.c
	gcc -ffreestanding -Wall -Wextra -Iinclude -g -c src/allocator.c

clean:
	rm -rf *.o test_allocator