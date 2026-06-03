// echo Simple Client in C
// This client connects to the echo server, sends a message, and prints the response.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 9000
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[]) {
    int sock_fd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *message = "Hello, Echo Server!";

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully.\n");
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT); // 포트 번호 설정
    // 서버 주소 설정 (localhost)
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    // 서버에 연결
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }
    printf("Connected to the server.\n");
    // 메시지 전송
    send(sock_fd, message, strlen(message), 0);
    printf("Message sent: %s\n", message);
    // 서버로부터 응답 수신
    int len = recv(sock_fd, buffer, BUFFER_SIZE, 0);
    if (len > 0) {
        buffer[len] = '\0'; // 문자열 종료 문자 추가
        printf("Message received from server: %s\n", buffer);
    } else {
        printf("Failed to receive message from server.\n"); 
    }
    close(sock_fd); // 소켓 닫기
    return 0;
}
