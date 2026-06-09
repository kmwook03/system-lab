#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 1024
#define MAX_SOCK 512

int readline(int fd, char *ptr, int maxlen) {
    int n, rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;

            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        }
    }

    *ptr = 0;
    return n;
}

char *escapechar = "exit\n";

int s; /* 서버와 연결된 소켓번호 */

struct Name {
    char n[20];   /* 대화방에서 사용할 이름 */
    int len;      /* 이름의 크기 */
} name;

int main(int argc, char *argv[]) {
    char line[MAXLINE], sendline[MAXLINE + 1];
    int size;

    struct sockaddr_in server_addr;

    int nfds;
    fd_set read_fds;

    if (argc < 4) {
        printf("실행방법 : %s 호스트 IP주소 포트번호 사용자이름 \n", argv[0]);
        return -1;
    }

    /* 채팅 참가자 이름 구조체 초기화 */
    sprintf(name.n, "[%s]", argv[3]);
    name.len = strlen(name.n);

    /* 소켓 생성 */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Client : Can't open stream socket.\n");
        return -1;
    }

    /* 채팅 서버의 소켓주소 구조체 server_addr 초기화 */
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    /* 연결요청 */
    if (connect(s, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        printf("Client : Can't connect to server.\n");
        return -1;
    }
    else {
        printf("접속에 성공했습니다..\n");
    }

    nfds = s + 1;

    FD_ZERO(&read_fds);

    while (1) {
        /* -------- select() 호출 ------------------------ */
        FD_SET(0, &read_fds);
        FD_SET(s, &read_fds);

        if (select(nfds, &read_fds,
                   (fd_set *)0,
                   (fd_set *)0,
                   (struct timeval *)0) < 0) {
            printf("select error\n");
            return -1;
        }

        /* ------ 서버로부터 수신한 메시지 처리 ------------- */
        if (FD_ISSET(s, &read_fds)) {
            char recvline[MAXLINE];
            int recv_size;

            if ((recv_size = recv(s, recvline, MAXLINE, 0)) > 0) {
                recvline[recv_size] = '\0';
                printf("%s \n", recvline);
            }
        }

        /* --------- 키보드 입력 처리 ------------------- */
        if (FD_ISSET(0, &read_fds)) {
            if (readline(0, sendline, MAXLINE) > 0) {
                size = strlen(sendline);

                sprintf(line, "%s %s", name.n, sendline);

                if (send(s, line, name.len + 1 + size, 0)
                    != (name.len + 1 + size)) {
                    printf("Error : Written error on socket.\n");
                }

                if (size == 5 &&
                    strncmp(sendline, escapechar, 5) == 0) {
                    printf("Good bye.\n");
                    close(s);
                    return -1;
                }
            }
        } /* end of 키보드 입력 처리 */
    } /* end of while() */
} /* end of main */
