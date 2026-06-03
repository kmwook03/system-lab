// ncurses - blinking example codes
#include <ncurses.h>
#include <unistd.h>

int main() {
    initscr();
    noecho();
    curs_set(FALSE);

    int x = 10;
    int y = 10;
    int ch;

    while (1) {
        clear(); // 화면 지움
        refresh(); // 지운 상태를 보여줌
        usleep(100000); // 100ms 대기 (지운 화면 오래 보여줌)
        mvprintw(y, x, "Blinking Text"); // 깜빡이는 텍스트 출력
        refresh(); // 텍스트가 보이도록 화면 갱신
        usleep(100000); // 100ms 대기 (텍스트 오래 보여줌)
        x++; // 텍스트 위치 이동
        if (x > COLS - 1) { // 화면 끝에 도달하면
            x = 0;
        }
    }


}