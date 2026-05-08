// pthread conditional variable example
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int data = 0;

void* producer(void* arg) {
    for (int i = 0; i < 5; i++) {
        sleep(1); // Simulate work
        pthread_mutex_lock(&mutex);
        data++;
        printf("Producer produced: %d\n", data);
        pthread_cond_signal(&cond); // Signal the consumer
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < 5; i++) {
        pthread_mutex_lock(&mutex);
        while (data == 0) { // Wait for data to be produced
            pthread_cond_wait(&cond, &mutex);
        }
        printf("Consumer consumed: %d\n", data);
        data--;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t prod_thread, cons_thread;

    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);

    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);

    return 0;
}
