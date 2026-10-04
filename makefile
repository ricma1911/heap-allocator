test_allocator: test_allocator.o allocator.o
	gcc test_allocator.o allocator.o -o test_allocator

test_allocator.o: tests/test_allocator.c
	gcc -Iinclude -c tests/test_allocator.c

allocator.o: src/allocator.c
	gcc -Iinclude -c src/allocator.c

clean:
	rm -rf *.o test_allocator