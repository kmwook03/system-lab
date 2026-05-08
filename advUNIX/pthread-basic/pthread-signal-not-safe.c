// pthread signal not safe example
// signal handler에서 printf()는 안전하지 않음

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void signal_handler(int signum) {
    // signal handler에서 printf()는 안전하지 않음
    pthread_mutex_lock(&mutex);
    printf("Received signal: %d\n", signum);
    pthread_mutex_unlock(&mutex);
}

int main() {
    // signal handler 등록
    signal(SIGINT, signal_handler);

    printf("Press Ctrl+C to trigger the signal handler...\n");

    int i = 0;
    while (1) {
        pthread_mutex_lock(&mutex);
        printf("Main thread: Running... %d\n", i);

        if (i++ >= 100) break; // 루프 종료 조건
        sleep(1);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}