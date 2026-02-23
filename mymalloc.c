#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEM_SIZE 4096
#define HEADER_SIZE 8

static union {
    char bytes[MEM_SIZE];
    double not_used;
} heap;

static int active = 0;

// Forward declaration
void detect_leaks();

// Leak detection function
void detect_leaks() {
    int leak_count = 0;
    size_t leak_bytes = 0;
    
    char *current = heap.bytes;
    while (current < heap.bytes + MEM_SIZE) {
        int *header = (int*)current;
        
        if (header[1] == 1) {  // Still allocated = leak
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

// Initialize heap
static void activate() {
    int *header = (int*)heap.bytes;
    header[0] = MEM_SIZE - HEADER_SIZE;
    header[1] = 0;
    active = 1;
    atexit(detect_leaks);  // Register leak detector
}

// Validate that ptr points to a valid chunk
static int is_valid_chunk(void *ptr) {
    if (ptr == NULL) return 0;
    
    char *p = (char*)ptr - HEADER_SIZE;
    
    if (p < heap.bytes || p >= heap.bytes + MEM_SIZE) {
        return 0;
    }
    
    // Walk through heap to verify this is a real chunk
    char *current = heap.bytes;
    while (current < heap.bytes + MEM_SIZE) {
        if (current == p) {
            return 1;  // Found it!
        }
        int *h = (int*)current;
        current = current + HEADER_SIZE + h[0];
    }
    
    return 0;  // Not found
}

void *mymalloc(size_t size, char *file, int line) {
    if (size == 0) {
        return NULL;
    }

    // Initialize on first use
    if (active == 0) {
        activate();
    }

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    // Search for free chunk
    char *start = heap.bytes;
    char *end = heap.bytes + MEM_SIZE;

    while (start < end) {
        int *hold = (int*)start;
        size_t storage = hold[0];
        int is_free = hold[1];

        // Found free chunk big enough?
        if (is_free == 0 && storage >= size) {

            // Split chunk if enough space left
            if (storage > size + HEADER_SIZE) {
                int *create = (int*)(start + HEADER_SIZE + size);
                create[0] = storage - size - HEADER_SIZE;
                create[1] = 0;
            }

            // Mark as allocated
            hold[1] = 1;
            hold[0] = (int)size;
            return (void*)(start + HEADER_SIZE);  // Return pointer to payload
        }

        // Move to next chunk
        start = start + HEADER_SIZE + storage;
    }

    // No space found
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", 
            size, file, line);
    return NULL;
}

void myfree(void *ptr, char *file, int line) {
    // NULL is OK to free
    if (ptr == NULL) {
        return;
    }

    // Validate pointer
    if (!is_valid_chunk(ptr)) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    int *header = (int*)((char*)ptr - HEADER_SIZE);
    
    // Check double free
    if (header[1] == 0) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
        exit(2);
    }

    char *end = heap.bytes + MEM_SIZE;

    // Mark as free
    header[1] = 0;

    // Coalesce with next chunk if free
    char *next = (char*)ptr + header[0];
    if (next < end) {
        int *next_header = (int*)next;
        if (next_header[1] == 0) {
            header[0] = header[0] + HEADER_SIZE + next_header[0];
        }
    }

    // Coalesce with previous chunk if free
    char *current = heap.bytes;
    while (current < (char*)header) {
        int *prev_header = (int*)current;
        char *next_chunk = current + HEADER_SIZE + prev_header[0];
        
        if (next_chunk == (char*)header) {  // Found chunk before us
            if (prev_header[1] == 0) {  // Previous is free
                prev_header[0] = prev_header[0] + HEADER_SIZE + header[0];
            }
            break;
        }
        
        current = next_chunk;
    }
}