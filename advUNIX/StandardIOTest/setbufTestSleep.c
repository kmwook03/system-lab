#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    char buf[1000] = {0};
    setbuf(stdout, buf);
    printf("Hello, ");
    sleep(1);
    printf("UNIX!\n");
    sleep(1);

    setbuf(stdout, NULL);
    printf("How ");
    sleep(1);
    printf("are ");
    sleep(1);
    printf("you?\n");
    return 0;
}
