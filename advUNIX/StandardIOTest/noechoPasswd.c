// unbuffered password input print * backspace
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>

int main() {
    struct termios old_termios, new_termios;
    char password[100];
    int i = 0;
    int ch;
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    printf("Enter password: ");
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == 127 || ch == 8) { // Handle backspace
            if (i > 0) {
                i--;
                printf("\b \b"); // Move back, print space, move back again
            }
        } else {
            password[i++] = ch;
            printf("*");
        }
    }
    password[i] = '\0';
    printf("\n");
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
    return 0;
}
