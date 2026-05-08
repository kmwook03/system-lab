// SIGALRM Repetition Test
// This test verifies that the SIGALRM signal is properly handled by the signal handler.
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

void handler(int signum) {
    printf("Clock Tick!!\n");
    return;
}

int main() {
    printf("Starting SIGALRM repetition test...\n");
    struct itimerval timer;
    signal(SIGALRM, handler);
    timer.it_value.tv_sec = 1; // Initial delay of 1 second
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1; // Repeat every 1 second
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL); // Set the timer to generate SIGALRM signals

    while (1)
        pause();
    
    return 0;
}
