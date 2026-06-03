#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void handle_client(int client_fd)
{
    char buffer[BUFFER_SIZE];

    // HTTP 요청 읽기
    int n = read(client_fd, buffer, sizeof(buffer) - 1);

    if (n <= 0)
    {
        close(client_fd);
        return;
    }

    buffer[n] = '\0';

    printf("\n===== HTTP REQUEST =====\n");
    printf("%s\n", buffer);

    // 간단한 GET 파싱
    char method[32];
    char path[256];

    sscanf(buffer, "%s %s", method, path);

    printf("Method: %s\n", method);
    printf("Path  : %s\n", path);

    // HTTP 응답 생성
    char body[1024];

    snprintf(body, sizeof(body),
             "<html>"
             "<body>"
             "<h1>Fork Web Server</h1>"
             "<p>PID: %d</p>"
             "<p>Path: %s</p>"
             "</body>"
             "</html>",
             getpid(),
             path);

    char response[4096];

    snprintf(response, sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "%s",
             strlen(body),
             body);

    // 응답 전송
    write(client_fd, response, strlen(response));

    close(client_fd);
}

// 좀비 프로세스 방지
void sigchld_handler(int signo)
{
    while (waitpid(-1, NULL, WNOHANG) > 0)
    {
    }
}

int main()
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    socklen_t client_len = sizeof(client_addr);

    // SIGCHLD 처리
    signal(SIGCHLD, sigchld_handler);

    // socket 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    // bind 재사용 옵션
    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    // listen
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("Fork HTTP Server Start : %d\n", PORT);

    while (1)
    {
        // 클라이언트 연결 수락
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("Client Connected\n");

        // 프로세스 생성
        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");

            close(client_fd);
            continue;
        }

        // Child Process
        if (pid == 0)
        {
            close(server_fd);

            handle_client(client_fd);

            exit(0);
        }

        // Parent Process
        close(client_fd);
    }

    close(server_fd);

    return 0;
}
