// SIGALRM Basic Test
// This test verifies that the SIGALRM signal is properly handled by the signal handler.
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

void handler(int signum) {
    printf("Received SIGALRM signal: %d\n", signum);
    return;
}

int main() {
    printf("Starting SIGALRM basic test...\n");
    signal(SIGALRM, handler);
    alarm(2); // Set an alarm to go off in 2 seconds
    printf("Alarm set for 2 seconds. Waiting for signal...\n");
    pause(); // Wait for the signal to be delivered
    printf("Signal received, exiting test.\n");
    return 0;
}
