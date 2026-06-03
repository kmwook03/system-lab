#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int daemon_init(void) {
    pid_t pid;
    if ((pid = fork()) < 0) return -1;
    else if (pid != 0) exit(0); // Parent process exits
    // Child process continues
    setsid(); // Create a new session
    chdir("/"); // Change working directory to root
    umask(0); // Reset file mode creation mask
    int max_fd = sysconf(_SC_OPEN_MAX);
    for (int fd = 0; fd < max_fd; fd++) 
        close(fd);
    open("/dev/null", O_RDWR); // Redirect stdin to /dev/null
    dup(0); // Redirect stdout to /dev/null
    dup(0); // Redirect stderr to /dev/null
    return 0;
}

int main() {
    daemon_init();

    while (1) {
        // Daemon process code here
        sleep(1); // Sleep to reduce CPU usage
    }

    return 0;
}
