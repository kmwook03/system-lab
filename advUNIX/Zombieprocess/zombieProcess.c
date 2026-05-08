// Make a simple zombie process in C
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid > 0) {
        printf("Parent process (PID: %d) created a child process (PID: %d)\n", getpid(), pid);
        sleep(40);
    } else if (pid == 0) {
        printf("Child process (PID: %d) is exiting...\n", getpid());
        exit(0); // Child process exits immediately, becoming a zombie
    } else {
        perror("fork failed");
        return 1;
    }

    return 0;
}
