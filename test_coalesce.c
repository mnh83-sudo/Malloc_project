#include <stdio.h>
#include "mymalloc.h"

void test_coalesce_two() {
    printf("Test 1: Coalesce two adjacent chunks\n");
    
    char *p1 = malloc(500);
    char *p2 = malloc(500);
    char *p3 = malloc(500);
    
    if (!p1 || !p2 || !p3) {
        printf("  FAIL: Initial allocation failed\n");
        return;
    }
    
    // Free first two - they should coalesce
    free(p1);
    free(p2);
    
    // Should now be able to allocate ~1000 bytes
    char *big = malloc(900);
    if (big == NULL) {
        printf("  FAIL: Coalescing didn't work\n");
        free(p3);
        return;
    }
    
    printf("  PASS\n\n");
    free(big);
    free(p3);
}

void test_coalesce_three() {
    printf("Test 2: Coalesce three adjacent chunks\n");
    
    char *p1 = malloc(400);
    char *p2 = malloc(400);
    char *p3 = malloc(400);
    char *p4 = malloc(400);
    
    if (!p1 || !p2 || !p3 || !p4) {
        printf("  FAIL: Initial allocation failed\n");
        return;
    }
    
    // Keep p1, free the rest
    free(p2);
    free(p3);
    free(p4);
    
    // Should coalesce into ~1200 bytes
    char *big = malloc(1100);
    if (big == NULL) {
        printf("  FAIL: Didn't coalesce all three\n");
        free(p1);
        return;
    }
    
    printf("  PASS\n\n");
    free(p1);
    free(big);
}

void test_coalesce_reverse() {
    printf("Test 3: Coalesce when freeing in reverse order\n");
    
    char *p1 = malloc(300);
    char *p2 = malloc(300);
    char *p3 = malloc(300);
    
    if (!p1 || !p2 || !p3) {
        printf("  FAIL: Initial allocation failed\n");
        return;
    }
    
    // Free in reverse order
    free(p3);
    free(p2);
    free(p1);
    
    // Should have one large free block
    char *big = malloc(800);
    if (big == NULL) {
        printf("  FAIL: Reverse coalescing didn't work\n");
        return;
    }
    
    printf("  PASS\n\n");
    free(big);
}

int main() {
    printf("=== Coalescing Tests ===\n\n");
    test_coalesce_two();
    test_coalesce_three();
    test_coalesce_reverse();
    printf("All coalescing tests completed!\n");
    return 0;
}
