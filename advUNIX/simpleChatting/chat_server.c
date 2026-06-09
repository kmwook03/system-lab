#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <netinet/in.h>

#define MAXLINE 1024
#define MAX_SOCK 512

char *escapechar = "exit\n";

int readline(int fd, char *ptr, int maxlen) {
    int n, rc;
    char c;

    for (n = 1; n < maxlen; n++) {
        if ((rc = read(fd, &c, 1)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (rc == 0) {
            if (n == 1)
                return 0;
            else
                break;
        }
    }

    *ptr = 0;
    return n;
}

/* ------------------------------- 종료문자 확인 함수 ----------------------------
   exitCheck()는 다음의 세 개의 인자를 필요로 한다
   rline: 클라이언트가 전송한 문자열 포인터
   escapechar: 종료문자 포인터
   len: 종료문자의 크기
   ----------------------------------------------------------------------------- */
int exitCheck(char *rline, char *escapechar, int len) {
    int i, max;
    char *tmp;

    max = strlen(rline);
    tmp = rline;

    for (i = 0; i < max; i++) {
        if (*tmp == escapechar[0]) {
            if (strncmp(tmp, escapechar, len) == 0)
                return 1;
        } else {
            tmp++;
        }
    }

    return -1;
}

int main(int argc, char *argv[]) {
    char rline[MAXLINE];
    char *start = "대화방에 오신걸 환영합니다...\n";

    int i, j, n;
    int s, client_fd;
    socklen_t clilen;
    int nfds;                  /* 최대 소켓번호 +1 */
    fd_set read_fds;           /* 읽기를 감지할 소켓번호 구조체 */
    int num_chat = 0;          /* 채팅 참가자 수 */

    /* 채팅에 참가하는 클라이언트들의 소켓번호 리스트 */
    int client_s[MAX_SOCK];

    struct sockaddr_in client_addr, server_addr;

    if (argc < 2) {
        printf("실행방법 :%s 포트번호\n", argv[0]);
        return -1;
    }

    printf("대화방 서버 초기화 중....\n");

    /* 초기소켓 생성 */
    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Server: Can't open stream socket.");
        return -1;
    }

    /* server_addr 구조체의 내용 세팅 */
    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    if (bind(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Server: Can't bind local address.\n");
        return -1;
    }

    /* 클라이언트로부터 연결요청을 기다림 */
    listen(s, 5);

    nfds = s + 1; /* 최대 소켓번호 +1 */

    FD_ZERO(&read_fds);

    while (1) {
        /* (최대 소켓번호 +1) 값을 갱신 */
        if ((num_chat - 1) >= 0)
            nfds = client_s[num_chat - 1] + 1;

        /* 읽기 변화를 감지할 소켓번호를 fd_set 구조체에 지정 */
        FD_SET(s, &read_fds);

        for (i = 0; i < num_chat; i++)
            FD_SET(client_s[i], &read_fds);

        /*---------------------------------- select() 호출 ------------------------------------- */
        if (select(nfds, &read_fds, (fd_set *)0, (fd_set *)0,
                   (struct timeval *)0) < 0) {
            printf("select error\n");
            return -1;
        }

        /*-------------------------- 클라이언트 연결요청 처리 --------------------------- */
        if (FD_ISSET(s, &read_fds)) {
            clilen = sizeof(client_addr);

            client_fd = accept(
                s,
                (struct sockaddr *)&client_addr,
                &clilen
            );

            if (client_fd != -1) {
                /* 채팅 클라이언트 목록에 추가 */
                client_s[num_chat] = client_fd;
                num_chat++;

                send(client_fd, start, strlen(start), 0);

                printf("%d번째 사용자 추가.\n", num_chat);
            }
        }

        /*- 임의의 클라이언트가 보낸 메시지를 모든 클라이언트에게 방송 - */
        for (i = 0; i < num_chat; i++) {
            if (FD_ISSET(client_s[i], &read_fds)) {
                if ((n = recv(client_s[i], rline, MAXLINE, 0)) > 0) {
                    rline[n] = '\0';

                    /* 종료문자 입력시 채팅 탈퇴 처리 */
                    if (exitCheck(rline, escapechar, 5) == 1) {
                        shutdown(client_s[i], 2);

                        if (i != num_chat - 1)
                            client_s[i] = client_s[num_chat - 1];

                        num_chat--;
                        continue;
                    }

                    /* 모든 채팅 참가자에게 메시지 방송 */
                    for (j = 0; j < num_chat; j++)
                        send(client_s[j], rline, n, 0);

                    printf("%s", rline);
                }
            }
        }
    }
}
