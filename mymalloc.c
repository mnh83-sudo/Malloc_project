#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 4096
#define HEADER_SIZE 8

static union {
    char bytes[MEM_SIZE];
    double not_used;
} heap;

static int active = 0;

void detect_leaks() {
    int leak_count = 0;
    size_t leak_bytes = 0;

    char *start = heap.bytes;
    char *end = heap.bytes + MEM_SIZE;

    while (start < end) {
        int *hold = (int*)start;
        int storage = hold[0];
        int is_free = hold[1];

        if (is_free != 0) {
            leak_count++;
            leak_bytes += storage;
        }

        start = start + HEADER_SIZE + storage;
    }

    if (leak_count > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n",
                leak_bytes, leak_count);
    }
}

/* storage[0] = payload size, storage[1] = 0 if free, 1 if allocated */
static void activate() {
    int *header = (int*)heap.bytes;
    header[0] = MEM_SIZE - HEADER_SIZE;
    header[1] = 0;
    active = 1;
    atexit(detect_leaks);
}

/* Walks the heap to confirm ptr matches a real chunk header.
   Catches invalid pointers, mid-chunk pointers, and non-heap pointers. */
static int is_valid_chunk(void *ptr) {
    if (ptr == NULL) return 0;

    char *p = (char*)ptr - HEADER_SIZE;

    if (p < heap.bytes || p >= heap.bytes + MEM_SIZE)
        return 0;

    char *start = heap.bytes;
    char *end = heap.bytes + MEM_SIZE;

    while (start < end) {
        if (start == p) return 1;
        int *hold = (int*)start;
        start = start + HEADER_SIZE + hold[0];
    }

    return 0;
}

void *mymalloc(size_t size, char *file, int line) {
    if (size == 0) {
        return NULL;
    }

    if (active == 0) {
        activate();
    }

    char *start = heap.bytes;
    char *end = heap.bytes + MEM_SIZE;
    size = (size + 7) & ~7;

    while (start < end) {
        int *hold = (int*)start;
        int storage = hold[0];
        int is_free = hold[1];

        if (is_free == 0 && storage >= (int)size) {

            if (storage > (int)size + HEADER_SIZE) {
                int *create = (int*)(start + HEADER_SIZE + size);
                create[0] = storage - size - HEADER_SIZE;
                create[1] = 0;
            }

            hold[1] = 1;
            hold[0] = (int)size;
            return (void*)(start + HEADER_SIZE);
        }

        start = start + HEADER_SIZE + storage;
    }

    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    if (ptr == NULL) return;

    if (!is_valid_chunk(ptr)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    int *header = (int*)((char*)ptr - HEADER_SIZE);

    if (header[1] == 0) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    char *end = heap.bytes + MEM_SIZE;
    header[1] = 0;

    // Block is at the start, only check next
    if ((char*)ptr - HEADER_SIZE == heap.bytes) {
        char *next = (char*)ptr + header[0];

        if (next < end) {
            int *next_header = (int*)next;
            if (next_header[1] == 0) {
                header[0] = header[0] + HEADER_SIZE + next_header[0];
            }
        }
    }

    // Block is at the end, only check previous
    else if ((char*)ptr + header[0] == end) {
        char *current = heap.bytes;
        int *prev_header = NULL;

        while (current < (char*)header) {
            prev_header = (int*)current;
            current = current + HEADER_SIZE + prev_header[0];
        }

        if (prev_header != NULL && prev_header[1] == 0) {
            prev_header[0] = prev_header[0] + HEADER_SIZE + header[0];
        }
    }

    // Block is in the middle, check both
    else {
        // check next first
        char *next = (char*)ptr + header[0];
        int *next_header = (int*)next;

        if (next_header[1] == 0) {
            // FIX: was "prev_header[0]" which didn't exist yet, now correctly "header[0]"
            header[0] = header[0] + HEADER_SIZE + next_header[0];
        }

        // then walk back to find previous
        char *current = heap.bytes;
        int *prev_header = NULL;

        while (current < (char*)header) {
            prev_header = (int*)current;
            current = current + HEADER_SIZE + prev_header[0];
        }

        if (prev_header != NULL && prev_header[1] == 0) {
            prev_header[0] = prev_header[0] + HEADER_SIZE + header[0];
        }
    }
}