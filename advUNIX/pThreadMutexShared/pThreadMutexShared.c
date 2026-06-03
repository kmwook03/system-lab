// multi process mutex.c
// Parent Child Process Mutex Example
// Compile with: gcc -o pThreadMutexShared pThreadMutexShared.c -lpthread
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <string.h>

#define NUM_ITERATIONS 5

typedef struct {
    pthread_mutex_t mutex;
    int counter;
} shared_data_t;

int main() {
    // Create shared memory for the mutex and counter
    // 이름이 없기 때문에 부모-자식 관계가 아닌 프로세스는 이 메모리를 공유할 수 없음
    shared_data_t* data = mmap(NULL, sizeof(shared_data_t),
        PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); // unnamed shared memory
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Initialize the mutex and counter
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED); // Mutex를 프로세스 간 공유하기 위한 설정 (리눅스만 가능)
    pthread_mutex_init(&data->mutex, &attr);
    data->counter = 0;

    pid_t pid = fork(); // 자식 프로세스 생성
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            pthread_mutex_lock(&data->mutex); // Mutex 잠금
            data->counter++;
            printf("Child Process: Counter = %d\n", data->counter);
            pthread_mutex_unlock(&data->mutex);
            sleep(1); // Simulate work
        }
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        for (int i = 0; i < NUM_ITERATIONS; i++) {
            pthread_mutex_lock(&data->mutex); // Mutex 잠금
            data->counter++;
            printf("Parent Process: Counter = %d\n", data->counter);
            pthread_mutex_unlock(&data->mutex);
            sleep(1); // Simulate work
        }
        wait(NULL); // Wait for child process to finish

        // Clean up
        pthread_mutex_destroy(&data->mutex);
        munmap(data, sizeof(shared_data_t));
    }
    return 0;
}
