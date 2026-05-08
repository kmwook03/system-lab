#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    fork();
    printf("Hello\n");
    sleep(10);
    return 0;
}
