// A simple shell implementation in C
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    char *token;
    int status;

    while (1) {
        // Print the shell prompt
        printf("myshell> ");
        fflush(stdout); // <---- 중요함!! 꼭 외울 것. 출력 버퍼를 비워서 프롬프트가 즉시 표시되도록 함

        // Read user input
        if (!fgets(input, sizeof(input), stdin)) { // fgets: file로 부터 문자열을 읽어옴
            perror("fgets");
            continue;
        }

        // Remove the newline character from the input
        input[strcspn(input, "\n")] = '\0';

        // Tokenize the input into arguments
        int arg_count = 0;
        token = strtok(input, " ");
        while (token != NULL && arg_count < MAX_ARGS - 1) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        args[arg_count] = NULL; // Null-terminate the arguments array <--- 여기까지 입력 처리

        // Check for built-in commands (e.g., exit)
        if (strcmp(args[0], "exit") == 0) {
            break; // Exit the shell
        }

        // Fork a child process to execute the command
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        } else if (pid == 0) {
            // In child process: execute the command
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // In parent process: wait for the child to finish
            wait(&status);
        }
    }

    return 0;
}
