#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main(int argc, char ** argv)
{
    pid_t   pid, waitPID;
    int     status, data = 10;

    if ((pid = fork()) < 0) // 1st fork
        perror("fork error");

    if (pid == 0) { //child
        if ((pid = fork()) < 0) // 2nd fork
            perror("fork error");
        else if (pid > 0) { // parent process for 2nd fork()
                printf("firat child, parent id: %d\n", getppid());
                printf("firat child, its own id: %d\n", getpid());
                exit(0);
            }
        // 2nd child process

        sleep(2);

        //as here

        // just add something that you want to do ,,, here

        data = data + 1;

        //

        printf("second chil. parentd id: %d\n", getppid());

        printf("second chil. its own id: %d\n", getpid());

        exit(0);

    }  



    waitPID = waitpid(pid, NULL, 0);  // pid => 1st child process



    if (waitPID != pid) // wait for child

        perror("Wait error");





    printf("parent process, parent id: %d\n", getppid());

    printf("parent process, its own id: %d\n", getpid());

    exit(0);

}

