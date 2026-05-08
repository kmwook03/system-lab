// Using USR1 USR2 signals to implement a semaphore in C
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

// volatile: 컴파일러 최적화 방지
// sig_atomic_t: 시그널 핸들러에서 안전하게 사용할 수 있는 정수 타입 (원자성 보장)
static volatile sig_atomic_t semaphore = 0;
static sigset_t nw;

void sig_handler(int sig) {}

void my_sem_wait() {
    sigset_t old_mask;
}
