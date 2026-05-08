// SIGNINT signal handling width pthreads
// SIGTERM signal handling example using pthreads
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

void *signal_thread(void *arg) {
    sigset_t *set = (sigset_t *)arg;
    int sig;

    while (1) {
        // Wait for a signal to be delivered
        if (sigwait(set, &sig) != 0) {
            perror("sigwait");
            exit(EXIT_FAILURE);
        }

        // Handle the received signal
        if (sig == SIGINT) {
            printf("Received SIGINT, exiting...\n");
            exit(EXIT_SUCCESS);
        } else if (sig == SIGTERM) {
            printf("Received SIGTERM, exiting...\n");
            exit(EXIT_SUCCESS);
        }
    }
}

void *worker_thread(void *arg) {
    while (1) {
        printf("Worker thread is running...\n");
        sleep(1);
    }
}

int main() {
    pthread_t thread;
    sigset_t set;

    // Initialize the signal set
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);

    // Block the signals in the main thread
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0) {
        perror("pthread_sigmask");
        exit(EXIT_FAILURE);
    }

    // Create a thread to handle signals
    if (pthread_create(&thread, NULL, signal_thread, (void *)&set) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    // Wait for the signal handling thread to finish (it won't in this example)
    pthread_join(thread, NULL);

    return 0;
}
