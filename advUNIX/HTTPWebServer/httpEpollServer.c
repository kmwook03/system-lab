#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <sys/epoll.h> // epoll 헤더

#define PORT 8080
#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

int main()
{
    int server_fd;

    struct sockaddr_in server_addr;

    // 1. socket 생성
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

    // 2. bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    // 3. listen
    if (listen(server_fd, 10) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("epoll HTTP Server Start : %d\n",
           PORT);

    // 4. epoll 생성
    int epfd = epoll_create1(0);

    if (epfd < 0)
    {
        perror("epoll_create1");
        exit(1);
    }

    // 5. listen socket 등록
    struct epoll_event ev;

    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    epoll_ctl(epfd,
              EPOLL_CTL_ADD,
              server_fd,
              &ev);

    struct epoll_event events[MAX_EVENTS];

    while (1)
    {
        // 6. 이벤트 대기
        int nfds = epoll_wait(epfd,
                              events,
                              MAX_EVENTS,
                              -1);

        if (nfds < 0)
        {
            perror("epoll_wait");
            continue;
        }

        // 7. 발생한 이벤트 처리
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;

            // 새 연결 요청
            if (fd == server_fd)
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

                // client socket epoll 등록
                struct epoll_event client_ev;

                client_ev.events = EPOLLIN;
                client_ev.data.fd = client_fd;

                epoll_ctl(epfd,
                          EPOLL_CTL_ADD,
                          client_fd,
                          &client_ev);
            }
            else
            {
                // 클라이언트 데이터 처리
                char buffer[BUFFER_SIZE];

                int n = read(fd,
                             buffer,
                             sizeof(buffer) - 1);

                if (n <= 0)
                {
                    printf("Client Disconnected : fd=%d\n",
                           fd);

                    close(fd);

                    epoll_ctl(epfd,
                              EPOLL_CTL_DEL,
                              fd,
                              NULL);

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
                    "<h1>epoll HTTP Server</h1>"
                    "</body>"
                    "</html>";

                write(fd,
                      response,
                      strlen(response));

                close(fd);

                epoll_ctl(epfd,
                          EPOLL_CTL_DEL,
                          fd,
                          NULL);
            }
        }
    }

    close(server_fd);

    return 0;
}
