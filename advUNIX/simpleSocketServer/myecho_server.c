#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define BUF_LEN 128

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr, client_addr;
    int server_fd, client_fd;
    int len, len_out;
    int port;
    char buf[BUF_LEN+1];

    if (argc != 2) {
        printf("Usage: %s port\n", argv[0]);
        return -1;
    }
    port = atoi(argv[1]);

    if ((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("can't create socket\n");
        return -1;
    }
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("can't bind to local address\n");
        return -1;
    }
    listen(server_fd, 5);

    while (1) {
        printf("Server: waiting for a connection...\n");
        len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
        if (client_fd < 0) {
            printf("accept error\n");
            return -1;
        }
        printf("Server: client connected\n");
        len_out = read(client_fd, buf, sizeof(buf));
        buf[len_out] = '\0';
        printf("Received string: %s\n", buf);
        write(client_fd, buf, len_out);
        close(client_fd);
    }
    close(server_fd);
    return 0;
}