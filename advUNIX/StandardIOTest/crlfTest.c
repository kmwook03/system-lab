// Carriage return and line feed test
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Hello World!\r\n");
    printf("Hello\rWorld!\r\n");  // It will print only "World!" because of the carriage return
    printf("End of test.\r\n");
    printf("This is a test of CRLF.\n");
    return 0;
}
