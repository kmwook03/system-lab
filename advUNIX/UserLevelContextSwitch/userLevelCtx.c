#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>

#define STACK_SIZE 8192
#define MAX_TASK 3
#define TIME_SLICE 5* 100000 // 500ms

// Task structure
typedef struct {
    ucontext_t context;
    int id;
    char stack[STACK_SIZE];
} TCB;

TCB taskQueue[MAX_TASK];
int readyQueue[MAX_TASK];
int currentTask = -1;
int front = 0, rear = 0;

void enqueue(int taskId) {
    readyQueue[rear] = taskId;
    rear = (rear + 1) % MAX_TASK;
}

int dequeue() {
    if (front == rear) {
        return -1; // Queue is empty
    }
    int taskId = readyQueue[front];
    front = (front + 1) % MAX_TASK;
    return taskId;
}

void scheduler(int signum) {
    int previousTask = currentTask;
    enqueue(previousTask);
    currentTask = dequeue();
    swapcontext(&taskQueue[previousTask].context, &taskQueue[currentTask].context);
}

void taskFunction(int id) {
    while (1) {
        printf("Running Task %d\n", id);
        fflush(stdout);
        pause(); // It need SIGALRM
    }
}

int main() {
    struct itimerval timer;
    // Initialize tasks
    for (int i = 0; i < MAX_TASK; i++) {
        taskQueue[i].id = i;
        getcontext(&taskQueue[i].context);
        taskQueue[i].context.uc_stack.ss_sp = taskQueue[i].stack;
        taskQueue[i].context.uc_stack.ss_size = sizeof(taskQueue[i].stack);
        taskQueue[i].context.uc_link = NULL;
        makecontext(&taskQueue[i].context, (void (*)(void))taskFunction, 1, i);
        enqueue(i);
    }

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = TIME_SLICE;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = TIME_SLICE;
    setitimer(ITIMER_REAL, &timer, NULL);
    signal(SIGALRM, scheduler);
    
    currentTask = dequeue();
    setcontext(&taskQueue[currentTask].context);

    return 0;
}
