test_allocator: test_allocator.o allocator.o
	gcc -std=c17 -Wall -Wextra test_allocator.o allocator.o -o test_allocator

test_allocator.o: tests/test_allocator.c
	gcc -std=c17 -Wall -Wextra -Iinclude -g -c tests/test_allocator.c

allocator.o: src/allocator.c
	gcc -std=c17 -ffreestanding -Wall -Wextra -Iinclude -g -c src/allocator.c

clean:
	rm -rf *.o test_allocator