// fork echo server that handles multiple clients concurrently
#define _GNU_SOURCE // for sigaction with SA_RESTART
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <signal.h>
#include <errno.h>

#define PORT 9000
#define BUFFER_SIZE 1024

// 자식 프로세스 종료를 감지하고 거둬들이는 시그널 핸들러
void sigchld_handler(int s) {
    // waitpid()가 전역 변수 errno를 덮어쓸 수 있으므로 백업 및 복원
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int main(int argc, char const *argv[]) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // SIGCHLD 시그널 핸들러 등록
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // 핸들러 함수 지정
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // 시스템 콜이 인터럽트 되면 재시작
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. 소켓 옵션 설정 (SO_REUSEADDR만 사용하여 호환성 향상)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // 4. Listen
    if (listen(server_fd, 5) < 0) { // 대기열 큐 크기를 3에서 5로 약간 늘림
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Waiting for a connection...\n");
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            continue; // 에러가 나도 서버가 죽지 않도록 continue 처리
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // 클라이언트를 처리할 자식 프로세스 생성
        pid_t pid = fork();
        
        if (pid == 0) { // Child process
            close(server_fd); // 자식은 리스닝 소켓이 필요 없으므로 닫음
            
            while (1) {
                memset(buffer, 0, BUFFER_SIZE);
                int valread = read(new_socket, buffer, BUFFER_SIZE);
                
                if (valread <= 0) {
                    printf("Client disconnected: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                    break;
                }
                
                printf("Received from client: %s\n", buffer);
                send(new_socket, buffer, valread, 0); // 메시지 에코
            }
            
            close(new_socket); // 클라이언트 소켓 닫기
            exit(0); // 자식 프로세스 안전하게 종료
            
        } else if (pid > 0) { // Parent process
            close(new_socket); // 부모는 클라이언트 통신 소켓을 닫고 다시 accept 대기
            // waitpid는 이제 sigchld_handler가 백그라운드에서 비동기적으로 처리합니다.
            
        } else {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    
    close(server_fd); 
    return 0;       
}
