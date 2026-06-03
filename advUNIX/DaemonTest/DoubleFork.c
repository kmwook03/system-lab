#define _GNU_SOURCE
#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void daemonize(const char *cmd) {
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    umask(0); // Clear file mode creation mask
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
        // err_quit("%s: can't get file limit", cmd);
        fprintf(stderr, "%s: can't get file limit\n", cmd);
        exit(1);
    }
    
    if ((pid = fork()) < 0) {
        fprintf(stderr, "%s: can't fork\n", cmd);
        exit(1);
    }
    else if (pid != 0) // Parent process
        exit(0);
    setsid(); // Become session leader

    sa.sa_handler = SIG_IGN; // Ignore SIGHUP
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0) {
        fprintf(stderr, "%s: can't ignore SIGHUP\n", cmd);
        exit(1);
    }
    if ((pid = fork()) < 0) // Double fork
        fprintf(stderr, "%s: can't fork\n", cmd);
    else if (pid != 0) // Parent process   
        exit(0);
    
    if (chdir("/") < 0) { // Change working directory to root
        fprintf(stderr, "%s: can't change directory to /\n", cmd);
        exit(1);
    }
    
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024; // Set maximum number of file descriptors
    for (i = 0; i < rl.rlim_max; i++)
        close(i); // Close all open file descriptors
    
    fd0 = open("/dev/null", O_RDWR); // Redirect stdin to /dev/null
    fd1 = dup(0); // Redirect stdout to /dev/null
    fd2 = dup(0); // Redirect stderr to /dev/null

    openlog(cmd, LOG_CONS, LOG_DAEMON); // Initialize syslog

    if (fd0 != 0 || fd1 != 1 || fd2 != 2) {
        syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
        exit(1);
    }
}

int main() {
    daemonize("DoubleFork");
    // Daemon code goes here
    while (1) {
        // Daemon is running
        sleep(10);
    }
    return 0;
}
