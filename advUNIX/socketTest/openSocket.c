#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    int fd1, fd2, sd1, sd2;
    fd1 = open("/etc/passwd", O_RDONLY, 0);
    printf("/etc/passwd fd: %d\n", fd1);

    sd1 = socket(PF_INET, SOCK_STREAM, 0);
    printf("stream socket sd: %d\n", sd1);

    sd2 = socket(PF_INET, SOCK_DGRAM, 0);
    printf("datagram socket sd: %d\n", sd2);

    fd2 = open("/etc/hosts", O_RDONLY, 0);
    printf("/etc/hosts fd: %d\n", fd2);

    close(fd1);
    close(fd2);
    close(sd1);
    close(sd2);
}
