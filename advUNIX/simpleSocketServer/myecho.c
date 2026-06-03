#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_LEN 128

int main(int argc, char *argv[]) {
    int s, n, len_in, len_out;
    struct sockaddr_in server_addr;
    char *haddr;
    char buf[BUF_LEN+1];
    if (argc != 2) {
        printf("Usage: %s ipaddress\n", argv[0]);
        return -1;
    }
    haddr = argv[1];
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("can't create socket\n");
        return -1;
    }
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(haddr);
    server_addr.sin_port = htons(7);

    if (connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("can't connect to server\n");
        return -1;
    }
    printf("input any string: ");

    if (fgets(buf, BUF_LEN, stdin)) {
        buf[BUF_LEN] = '\0';
        len_out = strlen(buf);
    } else {
        printf("fgets error\n");
        return -1;
    }

    if (write(s, buf, len_out) < 0) {
        printf("write error\n");
        return -1;
    }
    printf("Echoed string: ");
    for (len_in = 0; len_in < len_out; len_in += n) {
        if ((n = read(s, &buf[len_in], len_out - len_in)) < 0) {
            printf("read error\n");
            return -1;
        }
    }
    printf("%s", buf);
    close(s);
    return 0;
}
