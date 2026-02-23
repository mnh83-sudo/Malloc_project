#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEM_SIZE    4096
#define HEADER_SIZE 8      // Two ints: [0] = payload size, [1] = allocated flag (1) or free (0)

/* The union ensures heap.bytes starts at an 8-byte aligned address.
   heap.not_used is never accessed — it exists only to enforce alignment. */
static union {
    char bytes[MEM_SIZE];
    double not_used;
} heap;

static int active = 0;  // Tracks whether the heap has been initialized

void detect_leaks();

/* Walks the entire heap at program exit and reports any chunks still allocated. 
   Registered via atexit() — must not call exit() itself. */
void detect_leaks() {
    int leak_count = 0;
    size_t leak_bytes = 0;

    char *current = heap.bytes;
    while (current < heap.bytes + MEM_SIZE) {
        int *header = (int*)current;
        if (header[1] == 1) {
            leak_count++;
            leak_bytes += header[0];
        }
        current = current + HEADER_SIZE + header[0];
    }

    if (leak_count > 0) {
        fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n",
                leak_bytes, leak_count);
    }
}

/* Sets up the heap as one large free chunk on first use.
   Registers the leak detector to run at program exit. */
static void activate() {
    int *header = (int*)heap.bytes;
    header[0] = MEM_SIZE - HEADER_SIZE;
    header[1] = 0;
    active = 1;
    atexit(detect_leaks);
}

/* Walks the heap to confirm ptr is a valid payload pointer from mymalloc.
   Backs up HEADER_SIZE bytes and checks if that address matches any real chunk header.
   This catches invalid pointers, mid-chunk pointers, and non-heap pointers. */
static int is_valid_chunk(void *ptr) {
    if (ptr == NULL) return 0;

    char *p = (char*)ptr - HEADER_SIZE;

    if (p < heap.bytes || p >= heap.bytes + MEM_SIZE)
        return 0;

    char *current = heap.bytes;
    while (current < heap.bytes + MEM_SIZE) {
        if (current == p) return 1;
        int *h = (int*)current;
        current = current + HEADER_SIZE + h[0];
    }

    return 0;
}

/* Allocates at least 'size' bytes from the heap.
   Size is rounded up to the nearest multiple of 8 to maintain alignment.
   Splits the found chunk if leftover space is large enough for another chunk. */
void *mymalloc(size_t size, char *file, int line) {
    if (size == 0) return NULL;

    if (active == 0) activate();

    // Round up to nearest multiple of 8
    size = (size + 7) & ~7;

    char *start = heap.bytes;
    char *end   = heap.bytes + MEM_SIZE;

    while (start < end) {
        int *hold      = (int*)start;
        size_t storage = hold[0];
        int is_free    = hold[1];

        if (is_free == 0 && storage >= size) {
            // Split only if remainder can fit a full header + at least 8 bytes
            if (storage > size + HEADER_SIZE) {
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

/* Frees a previously allocated chunk and coalesces adjacent free chunks.
   Validates the pointer before freeing — exits with status 2 on any error. */
void myfree(void *ptr, char *file, int line) {
    if (ptr == NULL) return;

    if (!is_valid_chunk(ptr)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    int *header = (int*)((char*)ptr - HEADER_SIZE);

    // Double free check
    if (header[1] == 0) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    char *end = heap.bytes + MEM_SIZE;
    header[1] = 0;

    // Coalesce with next chunk if free
    char *next = (char*)ptr + header[0];
    if (next < end) {
        int *next_header = (int*)next;
        if (next_header[1] == 0)
            header[0] = header[0] + HEADER_SIZE + next_header[0];
    }

    // Walk back to find the previous chunk and coalesce if free
    char *current = heap.bytes;
    while (current < (char*)header) {
        int *prev_header = (int*)current;
        char *next_chunk = current + HEADER_SIZE + prev_header[0];

        if (next_chunk == (char*)header) {
            if (prev_header[1] == 0)
                prev_header[0] = prev_header[0] + HEADER_SIZE + header[0];
            break;
        }

        current = next_chunk;
    }
}