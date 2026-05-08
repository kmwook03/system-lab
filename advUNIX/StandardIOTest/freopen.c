#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

int main() {
    char *fname = "testfile.txt";
    int fd;

    printf("First printf is on the screen.\n");
    if ((fd = freopen(fname, "w", stdout)) == NULL) {
        perror("Failed to open file");
        exit(-1);
    }
    printf("Second printf is in the file.\n");
    fclose(stdout);
    return 0;
}
