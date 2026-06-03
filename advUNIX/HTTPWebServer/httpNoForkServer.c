#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080

int main()
{
    int server_fd;
    int client_fd;

    struct sockaddr_in server_addr;
    socklen_t addr_len = sizeof(server_addr);

    char buffer[4096];

    // 1. socket 생성
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    // 2. 주소 구조체 설정
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 3. bind
    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    // 4. listen
    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        exit(1);
    }

    printf("HTTP Server Start : port %d\n", PORT);

    while (1)
    {
        // 5. accept
        client_fd = accept(server_fd,
                           (struct sockaddr *)&server_addr,
                           &addr_len);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("Client Connected\n");

        // 6. HTTP 요청 읽기
        int n = read(client_fd, buffer, sizeof(buffer) - 1);

        if (n > 0)
        {
            buffer[n] = '\0';

            printf("=== HTTP Request ===\n");
            printf("%s\n", buffer);
        }

        // 7. HTTP 응답
        char response[] =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "\r\n"
            "<html>"
            "<body>"
            "<h1>Hello HTTP Advanced Unix Programming Class Server</h1>"
            "</body>"
            "</html>";

        write(client_fd, response, strlen(response));

        // 8. 연결 종료
        close(client_fd);
    }

    close(server_fd);

    return 0;
}
