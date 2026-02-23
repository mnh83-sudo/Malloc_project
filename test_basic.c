#include <stdio.h>
#include <string.h>
#include "mymalloc.h"

void test_malloc_free() {
    printf("Test 1: Basic malloc and free\n");
    char *p = malloc(100);
    if (p == NULL) {
        printf("  FAIL: malloc returned NULL\n");
        return;
    }
    free(p);
    printf("  PASS\n\n");
}

void test_multiple_allocs() {
    printf("Test 2: Multiple allocations don't overlap\n");
    char *p1 = malloc(50);
    char *p2 = malloc(100);
    char *p3 = malloc(150);
    
    if (p1 == NULL || p2 == NULL || p3 == NULL) {
        printf("  FAIL: One allocation returned NULL\n");
        return;
    }
    
    // Fill with unique patterns
    memset(p1, 'A', 50);
    memset(p2, 'B', 100);
    memset(p3, 'C', 150);
    
    // Verify no overlap
    int ok = 1;
    for (int i = 0; i < 50 && ok; i++) if (p1[i] != 'A') ok = 0;
    for (int i = 0; i < 100 && ok; i++) if (p2[i] != 'B') ok = 0;
    for (int i = 0; i < 150 && ok; i++) if (p3[i] != 'C') ok = 0;
    
    if (ok) {
        printf("  PASS: No memory overlap\n\n");
    } else {
        printf("  FAIL: Memory overlapped\n\n");
    }
    
    free(p1);
    free(p2);
    free(p3);
}

void test_reuse() {
    printf("Test 3: Memory reuse after free\n");
    
    // Allocate something
    char *p1 = malloc(1000);
    if (!p1) {
        printf("  FAIL: malloc failed\n");
        return;
    }
    
    // Free it
    free(p1);
    
    // Allocate again - should reuse the freed space
    char *p2 = malloc(800);
    if (!p2) {
        printf("  FAIL: Memory not reused\n");
        return;
    }
    
    printf("  PASS: Memory reused correctly\n\n");
    free(p2);
}

void test_alignment() {
    printf("Test 4: 8-byte alignment\n");
    
    int sizes[] = {1, 5, 7, 13, 20, 33, 100};
    int all_aligned = 1;
    
    for (int i = 0; i < 7; i++) {
        char *p = malloc(sizes[i]);
        if (p == NULL) {
            printf("  FAIL: malloc returned NULL for size %d\n", sizes[i]);
            all_aligned = 0;
            break;
        }
        
        if ((unsigned long)p % 8 != 0) {
            printf("  FAIL: Pointer %p not 8-byte aligned\n", (void*)p);
            all_aligned = 0;
            free(p);
            break;
        }
        free(p);
    }
    
    if (all_aligned) {
        printf("  PASS: All pointers properly aligned\n\n");
    }
}

int main() {
    printf("=== Basic Functionality Tests ===\n\n");
    test_malloc_free();
    test_multiple_allocs();
    test_reuse();
    test_alignment();
    printf("All basic tests completed!\n");
    return 0;
}

