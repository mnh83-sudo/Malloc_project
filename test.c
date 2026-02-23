#include <stdio.h>
#include "mymalloc.h"

int main() {
    printf("Testing malloc...\n");
    
    char *p = malloc(100);
    if (p == NULL) {
        printf("FAIL: malloc returned NULL\n");
        return 1;
    }
    
    printf("SUCCESS: malloc returned pointer %p\n", (void*)p);
    
    printf("Writing to memory...\n");
    for (int i = 0; i < 100; i++) {
        p[i] = 'A';
    }
    
    printf("SUCCESS: Wrote to memory\n");
    
    printf("Freeing memory...\n");
    free(p);
    
    printf("SUCCESS: Freed memory\n");
    printf("All tests passed!\n");
    
 
   return 0;
}
