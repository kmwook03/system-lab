// stress.c
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define CHUNK_SIZE (10 * 1024 * 1024) // 10MB 단위로 할당

int main() {
    printf("=> 메모리 스트레스 테스트 시작...\n");
    // 100MB 할당 요청 (Virtual Memory)
    char *ptr = NULL;
    int allocated = 0;
    
    while (1) {
        char *temp = realloc(ptr, allocated + CHUNK_SIZE);
        if (!temp) {
            perror("=> [Error] 가상 메모리 할당 실패");
            break;
        }
        ptr = temp;

        memset(ptr + allocated, 1, CHUNK_SIZE); // 물리 메모리 할당
        allocated += CHUNK_SIZE;
        printf("=> 할당된 물리 메모리: %d MB\n", allocated / (1024 * 1024));

        usleep(500000); // 0.5초 대기
    }
    
    printf("=> [Fatal] Cgroup 제한 실패");
    free(ptr);
    return 0;
}
