// USR1 USR2 Parent Child Process Synchronization using flag
// signal이 queuing 되는지 확인
// Ctrl+C를 눌렀을 때 중첩이 되는지 확인하라는 뜻 -> signal handler가 중첩 실행 되는가?

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

static volatile sig_atomic_t sigflag; // set nonzero by sig handler()
static sigset_t newmask, oldmask, zeromask;

static void sig_usr(int signo) {
    sigflag = 1;
}

void charattime(char *str) {
    char *ptr;
    int c;
    setbuf(stdout, NULL); // unbuffered
    for (ptr = str; (c = *ptr++) != 0; ) {
        putc(c, stdout);
    }
}

void TELL_WAIT() {
    if (signal(SIGUSR1, sig_usr) == SIG_ERR) {
        perror("signal(SIGUSR1) error");
        exit(1);
    }
    if (signal(SIGUSR2, sig_usr) == SIG_ERR) {
        perror("signal(SIGUSR2) error");
        exit(1);
    }
    // signal mask 초기화
    sigemptyset(&zeromask);
    sigemptyset(&newmask);
    // newmask에 SIGUSR1, SIGUSR2 추가
    sigaddset(&newmask, SIGUSR1);
    sigaddset(&newmask, SIGUSR2);
    // SIGUSR1, SIGUSR2 블록
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0) {
        perror("SIG_BLOCK error");
        exit(1);
    }
}

void TELL_PARENT(pid_t pid) {
    kill(pid, SIGUSR2); // 부모 프로세스에게 SIGUSR2 신호 전송
}

void WAIT_PARENT() {
    while (sigflag == 0) {
        sigsuspend(&zeromask); // 신호가 올 때까지 대기
    }
    sigflag = 0; // 신호 처리 후 플래그 초기화
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        perror("SIG_SETMASK error");
        exit(1);
    }
}

void TELL_CHILD(pid_t pid) {
    kill(pid, SIGUSR1); // 자식 프로세스에게 SIGUSR1 신호 전송
}

void WAIT_CHILD() {
    while (sigflag == 0) {
        sigsuspend(&zeromask); // 신호가 올 때까지 대기
    }
    sigflag = 0; // 신호 처리 후 플래그 초기화
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0) {
        perror("SIG_SETMASK error");
        exit(1);
    }
}

int main() {
    pid_t pid;
    char str[100] = {0};

    // Initialize signal mask and handler
    TELL_WAIT();
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(1);
    } else if (pid > 0) { // Parent process
        for (int i = 0; i < 10; i++) {
            charattime("Output from parent\n");
            TELL_CHILD(pid); // 자식 프로세스에게 신호 전송
            WAIT_CHILD(); // 자식 프로세스의 신호 대기
        }
        exit(0);
    } else { // Child process
        for (int i = 0; i < 10; i++) {
            WAIT_PARENT(); // 부모 프로세스의 신호 대기
            charattime("Output from child\n");
            TELL_PARENT(getppid()); // 부모 프로세스에게 신호 전송
        }
        exit(0);
    }

    return 0;
}
