#include <stdio.h>
#include "mymalloc.h"

int main() {
    printf("=== Error Detection Tests ===\n");
    printf("Each test should print an error and exit.\n");
    printf("Uncomment ONE test at a time in the code.\n\n");
    
    // TEST 1: Invalid pointer
    // printf("Test 1: Free invalid pointer\n");
    // int x = 5;
    // free(&x);
    
    // TEST 2: Pointer not at chunk start
    // printf("Test 2: Free pointer in middle of chunk\n");
    // char *p = malloc(100);
    // free(p + 16);
    
    // TEST 3: Double free
    printf("Test 3: Double free\n");
    char *p = malloc(100);
    free(p);
    free(p);  // Should error
    
    printf("FAIL: Should have exited!\n");
    return 0;
}
