#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include "mymalloc.h"

// Workload 1: malloc and immediately free 1-byte object, 120 times
void workload1() {
    for (int i = 0; i < 120; i++) {
        char *p = malloc(1);
        free(p);
    }
}

// Workload 2: malloc 120 1-byte objects, then free all
void workload2() {
    char *ptrs[120];
    for (int i = 0; i < 120; i++) {
        ptrs[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(ptrs[i]);
    }
}

// Workload 3: Random alloc/free until 120 allocations done
void workload3() {
    char *ptrs[120];
    int count = 0;
    int allocated = 0;
    
    while (allocated < 120) {
        if (rand() % 2 == 0 || count == 0) {
            // Allocate
            ptrs[count++] = malloc(1);
            allocated++;
        } else {
            // Free random
            int idx = rand() % count;
            free(ptrs[idx]);
            ptrs[idx] = ptrs[--count];
        }
    }
    
    // Free all remaining
    for (int i = 0; i < count; i++) {
        free(ptrs[i]);
    }
}

// Workload 4: Simulate linked list operations
void workload4() {
    typedef struct node {
        int data;
        struct node *next;
    } node_t;
    
    node_t *head = NULL;
    
    // Create 50 nodes
    for (int i = 0; i < 50; i++) {
        node_t *new_node = malloc(sizeof(node_t));
        new_node->data = i;
        new_node->next = head;
        head = new_node;
    }
    
    // Free all nodes
    while (head != NULL) {
        node_t *temp = head;
        head = head->next;
        free(temp);
    }
}

// Workload 5: Allocate varying sizes
void workload5() {
    void *ptrs[60];
    int sizes[] = {1, 8, 16, 32, 64, 128};
    
    // Allocate varying sizes
    for (int i = 0; i < 60; i++) {
        ptrs[i] = malloc(sizes[i % 6]);
    }
    
    // Free all
    for (int i = 0; i < 60; i++) {
        free(ptrs[i]);
    }
}

int main() {
    srand(time(NULL));  // Seed random number generator
    
    struct timeval start, end;
    
    printf("Running stress test...\n");
    gettimeofday(&start, NULL);
    
    // Run all workloads 50 times
    for (int i = 0; i < 50; i++) {
        workload1();
        workload2();
        workload3();
        workload4();
        workload5();
    }
    
    gettimeofday(&end, NULL);
    
    // Calculate elapsed time
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_usec - start.tv_usec) / 1000000.0;
    
    printf("Total time: %f seconds\n", elapsed);
    printf("Average time per run: %f seconds\n", elapsed / 50.0);
    
    return 0;
}
