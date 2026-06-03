#include <ncurses.h>
#include <string.h>

int main() {
    int x, y;
    int max_y, max_x;
    int ch;
    
    // 귀여운 캐릭터 모양 정의
    const char* char_right = "(>'-')>";  // 오른쪽을 볼 때
    const char* char_left  = "<('-'<)";  // 왼쪽을 볼 때
    const char* char_idle  = "( ^_^ )";  // 가만히 있거나 위/아래로 움직일 때
    
    const char* current_char = char_idle;

    // ncurses 초기화 설정
    initscr();             // ncurses 모드 시작
    noecho();              // 입력한 키보드 문자가 화면에 보이지 않도록 설정
    cbreak();              // 엔터를 누르지 않아도 즉시 입력받도록 설정
    keypad(stdscr, TRUE);  // 방향키 등 특수 키 입력을 가능하게 설정
    curs_set(0);           // 화면에서 깜빡이는 커서를 숨김

    // 현재 터미널 창의 최대 크기를 가져옴
    getmaxyx(stdscr, max_y, max_x);

    // 캐릭터의 시작 위치 (화면 정중앙)
    y = max_y / 2;
    x = (max_x - strlen(current_char)) / 2;

    while (1) {
        clear(); // 화면 전체 지우기
        
        // 화면 상단에 조작법 출력
        mvprintw(0, 0, "Move: Arrow Keys | Quit: 'q'"); 
        
        // 캐릭터 출력 (y축, x축, 출력할 문자열)
        mvprintw(y, x, "%s", current_char);
        
        refresh(); // 변경된 화면 내용을 실제 화면에 갱신

        ch = getch(); // 사용자의 키 입력 대기

        // 'q' 또는 'Q'를 누르면 루프 종료
        if (ch == 'q' || ch == 'Q') {
            break;
        }

        // 터미널 창 크기가 변경되었을 수 있으므로 최대 크기 다시 계산
        getmaxyx(stdscr, max_y, max_x);

        // 입력된 키에 따른 이동 및 화면 경계(벽) 충돌 방지 처리
        switch (ch) {
            case KEY_UP:
                if (y > 1) y--; // 맨 윗줄(조작법)을 침범하지 않도록 y > 1
                current_char = char_idle;
                break;
            case KEY_DOWN:
                if (y < max_y - 1) y++;
                current_char = char_idle;
                break;
            case KEY_LEFT:
                if (x > 0) x--;
                current_char = char_left; // 왼쪽을 보는 모습으로 변경
                break;
            case KEY_RIGHT:
                if (x < max_x - strlen(current_char)) x++;
                current_char = char_right; // 오른쪽을 보는 모습으로 변경
                break;
        }
    }

    // 프로그램 종료 전 ncurses 모드 해제 (터미널 설정 복구)
    endwin();
    
    return 0;
}

