#include <stdio.h>
#define MAX 5

// -------------------- Task 상태 --------------------
typedef enum {
    READY,
    BLOCKED
} state;

// -------------------- Task --------------------
typedef struct {
    int id;
    state st;
} Task;

// -------------------- Semaphore --------------------
typedef struct {
    int value;
    int queue[MAX];
    int front, rear;
} Semaphore;

Task tasks[MAX];
Semaphore sem;

// -------------------- Queue --------------------
void enqueue(int id) {
    sem.queue[sem.rear] = id;
    sem.rear = (sem.rear + 1) % MAX;
}

int dequeue() {
    int id = sem.queue[sem.front];
    sem.front = (sem.front + 1) % MAX;
    return id;
}

// -------------------- Semaphore --------------------
void sem_init(int val) {
    sem.value = val;
    sem.front = sem.rear = 0;
}

void sem_wait(int id) {
    printf("\n[Task %d] sem_wait()\n", id);
    sem.value--;

    if (sem.value < 0) {
        printf("→ Task %d BLOCKED\n", id);
        tasks[id].st = BLOCKED;
        enqueue(id);
    } else {
        printf("→ Task %d ENTER CS\n", id);
        tasks[id].st = READY;
    }

    printf("Semaphore value: %d\n", sem.value);
}

void sem_post() {
    printf("\n[sem_post()]\n");

    sem.value++;

    if (sem.value <= 0) {
        int id = dequeue();
        printf("→ Task %d WAKE UP (READY)\n", id);
        tasks[id].st = READY;
    }

    printf("Semaphore value: %d\n", sem.value);
}

// -------------------- 상태 출력 --------------------
void print_tasks(int n) {

    printf("\n[Task States]\n");

    for (int i = 0; i < n; i++) {
        printf("Task %d : %s\n", i,
               tasks[i].st == READY ? "READY" : "BLOCKED");
    }
}

// -------------------- Main --------------------
int main() {
    int n = 3;
    // 초기화
    for (int i = 0; i < n; i++) {
        tasks[i].id = i;
        tasks[i].st = READY;
    }
    sem_init(1);  // binary semaphore
    print_tasks(n);
    // 시뮬레이션 시나리오
    sem_wait(0);  // Task 0 진입
    sem_wait(1);  // Task 1 BLOCK
    sem_wait(2);  // Task 2 BLOCK
    print_tasks(n);
    sem_post();   // Task 1 깨어남
    sem_post();   // Task 2 깨어남
    print_tasks(n);

    return 0;
}
