CC = gcc
CFLAGS = -Wall -Wextra -g

all: memgrind test_basic test_coalesce test_errors memtest

memgrind: memgrind.c mymalloc.c
	$(CC) $(CFLAGS) -o memgrind memgrind.c mymalloc.c

memtest: memtest.c mymalloc.c
	$(CC) $(CFLAGS) -o memtest memtest.c mymalloc.c

test_basic: test_basic.c mymalloc.c
	$(CC) $(CFLAGS) -o test_basic test_basic.c mymalloc.c

test_coalesce: test_coalesce.c mymalloc.c
	$(CC) $(CFLAGS) -o test_coalesce test_coalesce.c mymalloc.c

test_errors: test_errors.c mymalloc.c
	$(CC) $(CFLAGS) -o test_errors test_errors.c mymalloc.c

clean:
	rm -f memgrind test_basic test_coalesce test_errors test *.o

.PHONY: all clean
