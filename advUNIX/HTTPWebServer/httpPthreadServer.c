#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void *client_thread(void *arg)
{
    int client_fd = *(int *)arg;

    free(arg);

    char buffer[BUFFER_SIZE];

    // HTTP 요청 읽기
    int n = read(client_fd,
                 buffer,
                 sizeof(buffer) - 1);

    if (n <= 0)
    {
        close(client_fd);
        return NULL;
    }

    buffer[n] = '\0';

    printf("\n===== HTTP REQUEST =====\n");
    printf("%s\n", buffer);

    // GET 파싱
    char method[32];
    char path[256];

    sscanf(buffer, "%s %s", method, path);

    printf("Thread ID : %ld\n",
           (long)pthread_self());

    printf("Method    : %s\n", method);
    printf("Path      : %s\n", path);

    // HTML Body
    char body[1024];

    snprintf(body,
             sizeof(body),
             "<html>"
             "<body>"
             "<h1>Pthread HTTP Server</h1>"
             "<p>Thread ID: %ld</p>"
             "<p>Path: %s</p>"
             "</body>"
             "</html>",
             (long)pthread_self(),
             path);

    // HTTP Response
    char response[4096];

    snprintf(response,
             sizeof(response),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %ld\r\n"
             "\r\n"
             "%s",
             strlen(body),
             body);

    // 응답 전송
    write(client_fd,
          response,
          strlen(response));

    close(client_fd);

    return NULL;
}

int main()
{
    int server_fd;

    struct sockaddr_in server_addr;

    // socket 생성
    server_fd = socket(AF_INET,
                       SOCK_STREAM,
                       0);

    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    // bind 재사용
    int opt = 1;

    setsockopt(server_fd,
               SOL_SOCKET,
               SO_REUSEADDR,
               &opt,
               sizeof(opt));

    memset(&server_addr,
           0,
           sizeof(server_addr));

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
    printf("Pthread HTTP Server Start : %d\n",
           PORT);

    while (1)
    {
        int client_fd;

        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        // 연결 수락
        client_fd = accept(server_fd,
                           (struct sockaddr *)&client_addr,
                           &client_len);
        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }
        printf("Client Connected\n");

        // client fd 전달용 동적 메모리
        int *pclient = malloc(sizeof(int));
        *pclient = client_fd;

        // thread 생성
        pthread_t tid;
        if (pthread_create(&tid,
                           NULL,
                           client_thread,
                           pclient) != 0)
        {
            perror("pthread_create");
            close(client_fd);
            free(pclient);
            continue;
        }
        // thread 종료 자동 회수
        pthread_detach(tid);
    }
    close(server_fd);
    return 0;
}
