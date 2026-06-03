// Simple Echo Socket Server in C
// This server listens for incoming connections and echoes back any received messages.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    int len;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // 주소 여러 개 중 아무거나 할당
    server_addr.sin_port = htons(PORT); // 포트 번호 설정 (호스트 바이트 순서를 네트워크 바이트 순서(Big-Endian)로 변환)
    // 서버 소켓에 주소와 포트 번호를 바인딩
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Bind successful.\n");
    // Listen
    if (listen(server_fd, 3) < 0) { // BACKLOG 3: 최대 3개의 연결 대기
        perror("listen failed");
        exit(EXIT_FAILURE);
    }
    printf("Listening on port %d...\n", PORT);

    // Accept incoming connections
    client_len = sizeof(client_addr);
    if ((client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) { // accept하면서 새로운 socket 반환
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    while(1) {
        memset(buffer, 0, BUFFER_SIZE); // 버퍼 초기화
        len = recv(client_fd, buffer, BUFFER_SIZE, 0); // 클라이언트로부터 메시지 수신
        if (len <= 0) {
            printf("Client disconnected.\n");
            break; // 클라이언트가 연결을 종료했거나 오류 발생
        }
        printf("Received message: %s\n", buffer);
        send(client_fd, buffer, len, 0); // 받은 메시지를 클라이언트로 다시 전송 (에코)
    }
    close(client_fd); // 클라이언트 소켓 닫기
    close(server_fd); // 서버 소켓 닫기
    return 0;
}