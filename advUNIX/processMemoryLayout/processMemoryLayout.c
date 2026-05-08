// print process memory layout
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_var = 10;

void sample_function() {
    // This function is just to have some code in the text segment
}

int main() {
    int local_var = 20;
    char *heap_var = malloc(100);
    if (heap_var == NULL) {
        perror("malloc");
        return 1;
    }

    printf("Text segment (code): %p\n", (void *)sample_function);
    printf("BSS segment (uninitialized global variable): %p\n", (void *)&main);
    printf("Data segment (global variable): %p\n", (void *)&global_var);
    printf("Stack segment (local variable): %p\n", (void *)&local_var);
    printf("Heap segment (dynamically allocated memory): %p\n", (void *)heap_var);
    
    free(heap_var);
    return 0;
}
