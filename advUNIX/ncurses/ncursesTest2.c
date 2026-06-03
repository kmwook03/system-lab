#include <ncurses.h>
#include <string.h>

// 동시에 화면에 존재할 수 있는 최대 총알 개수
#define MAX_BULLETS 10

// 총알 구조체 정의
typedef struct {
    int x, y;
    int active; // 1이면 화면에 존재(활성화), 0이면 비활성화
    int dir;    // 1이면 오른쪽으로 이동, -1이면 왼쪽으로 이동
} Bullet;

int main() {
    int x, y;
    int max_y, max_x;
    int ch;
    int facing = 1; // 캐릭터가 바라보는 방향 (1: 오른쪽, -1: 왼쪽)
    
    const char* char_right = "(>'-')>";
    const char* char_left  = "<('-'<)";
    const char* current_char = char_right;

    Bullet bullets[MAX_BULLETS] = {0}; // 총알 배열 초기화

    // ncurses 초기화
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    // 핵심 포인트: getch()가 입력 대기를 하지 않고 일정 시간(ms) 후 다음 코드로 넘어가게 함
    // 이렇게 해야 키를 누르지 않아도 총알이 계속 날아갈 수 있습니다. (50ms마다 화면 갱신)
    timeout(50); 

    getmaxyx(stdscr, max_y, max_x);

    y = max_y / 2;
    x = (max_x - strlen(current_char)) / 2;

    while (1) {
        clear(); // 화면 지우기
        
        // 조작법 출력
        mvprintw(0, 0, "Move: Arrow Keys | Shoot: Spacebar | Quit: 'q'"); 
        
        // 1. 총알 이동 및 출력
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (bullets[i].active) {
                bullets[i].x += bullets[i].dir * 2; // 방향에 따라 2칸씩 이동 (속도)
                
                // 화면 밖으로 나가면 총알 비활성화
                if (bullets[i].x < 0 || bullets[i].x >= max_x) {
                    bullets[i].active = 0;
                } else {
                    // 총알 모양 'o' 출력
                    mvprintw(bullets[i].y, bullets[i].x, "o");
                }
            }
        }

        // 2. 캐릭터 출력
        mvprintw(y, x, "%s", current_char);
        
        refresh(); // 화면 갱신

        ch = getch(); // 입력 확인 (timeout 덕분에 입력이 없으면 ERR 반환 후 루프 계속 진행)

        if (ch == 'q' || ch == 'Q') {
            break;
        }

        getmaxyx(stdscr, max_y, max_x);

        // 입력이 들어왔을 때만 처리
        if (ch != ERR) {
            switch (ch) {
                case KEY_UP:
                    if (y > 1) y--;
                    break;
                case KEY_DOWN:
                    if (y < max_y - 1) y++;
                    break;
                case KEY_LEFT:
                    if (x > 0) x--;
                    current_char = char_left;
                    facing = -1; // 왼쪽을 바라봄
                    break;
                case KEY_RIGHT:
                    if (x < max_x - strlen(current_char)) x++;
                    current_char = char_right;
                    facing = 1;  // 오른쪽을 바라봄
                    break;
                case ' ': // 스페이스바를 눌렀을 때 (총알 발사)
                    for (int i = 0; i < MAX_BULLETS; i++) {
                        // 비활성화된 빈 총알 슬롯을 찾아서 발사
                        if (!bullets[i].active) {
                            bullets[i].active = 1;
                            bullets[i].y = y;
                            bullets[i].dir = facing;
                            
                            // 캐릭터가 바라보는 방향에 맞춰 총알 생성 위치 조정
                            if (facing == 1) {
                                bullets[i].x = x + strlen(current_char);
                            } else {
                                bullets[i].x = x - 1;
                            }
                            break; // 한 번에 한 발만 쏘기 위해 루프 탈출
                        }
                    }
                    break;
            }
        }
    }

    endwin();
    return 0;
}
