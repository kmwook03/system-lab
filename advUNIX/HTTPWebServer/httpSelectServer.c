#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX_CLIENTS 1024

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

    printf("select HTTP Server Start : %d\n",
           PORT);

    // client socket 저장
    int clients[MAX_CLIENTS];

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = -1;
    }

    while (1)
    {
        fd_set readfds;

        FD_ZERO(&readfds);

        // listen socket 추가
        FD_SET(server_fd,
               &readfds);

        int maxfd = server_fd;

        // client socket 추가
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];

            if (fd != -1)
            {
                FD_SET(fd, &readfds);

                if (fd > maxfd)
                {
                    maxfd = fd;
                }
            }
        }

        // 이벤트 대기
        int activity = select(maxfd + 1,
                              &readfds,
                              NULL,
                              NULL,
                              NULL);

        if (activity < 0)
        {
            perror("select");
            continue;
        }

        // 새 연결 요청
        if (FD_ISSET(server_fd,
                     &readfds))
        {
            int client_fd;

            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            client_fd = accept(server_fd,
                               (struct sockaddr *)&client_addr,
                               &client_len);

            if (client_fd < 0)
            {
                perror("accept");
                continue;
            }

            printf("Client Connected : fd=%d\n",
                   client_fd);

            // 빈 슬롯 저장
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == -1)
                {
                    clients[i] = client_fd;
                    break;
                }
            }
        }

        // client 데이터 처리
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = clients[i];

            if (fd == -1)
            {
                continue;
            }

            // 읽기 가능한가?
            if (FD_ISSET(fd,
                         &readfds))
            {
                char buffer[BUFFER_SIZE];

                int n = read(fd,
                             buffer,
                             sizeof(buffer) - 1);

                // 연결 종료
                if (n <= 0)
                {
                    printf("Client Disconnected : fd=%d\n",
                           fd);

                    close(fd);

                    clients[i] = -1;

                    continue;
                }

                buffer[n] = '\0';

                printf("\n===== HTTP REQUEST =====\n");
                printf("%s\n", buffer);

                // HTTP 응답
                char response[] =
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "\r\n"
                    "<html>"
                    "<body>"
                    "<h1>Select HTTP Server</h1>"
                    "</body>"
                    "</html>";

                write(fd,
                      response,
                      strlen(response));

                close(fd);

                clients[i] = -1;
            }
        }
    }

    close(server_fd);

    return 0;
}
