#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    int fd1, fd2;

    fd1 = open("testfile.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (fd1 < 0) {
        perror("Failed to open file");
        return EXIT_FAILURE;
    }
    printf("Original file descriptor: %d\n", fd1);
    // 파일 디스크립터 복제
    fd2 = dup(fd1);
    if (fd2 < 0) {
        perror("Failed to duplicate file descriptor");
        close(fd1);
        return EXIT_FAILURE;
    }
    printf("Duplicated file descriptor: %d\n", fd2);
    write(fd1, "Hello, World!\n", 14);
    write(fd2, "This is a test.\n", 17);
    dup2(fd1, STDOUT_FILENO);
    printf("This will be written to the file instead of standard output.\n");
    close(fd1);
    close(fd2);
    return EXIT_SUCCESS;
}
