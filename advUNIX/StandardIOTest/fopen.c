#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;

    if ((fp = fopen("testfile.txt", "r")) == NULL) {
        perror("Failed to open file");
        exit(-1);
    }
    printf("Success!\n");
    printf("Opening \"testfile.txt\" in \"r\" mode!\n");
    fclose(fp);

    return 0;
}
