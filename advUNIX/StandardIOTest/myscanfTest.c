#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    printf("Enter a string: ");
    fflush(stdout);
    char buffer[100];
    setbuf(stdin, buffer); // Set the buffer for stdoin
    scanf("%s", buffer);
    printf("You entered: %s\n", buffer);

    return 0;
}
