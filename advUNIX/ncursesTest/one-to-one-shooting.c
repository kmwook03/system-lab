#include <ncurses.h>

#include <pthread.h>

#include <unistd.h>

#include <stdlib.h>

#include <string.h>

#include <fcntl.h>

#include <sys/mman.h>

#include <semaphore.h>



#define SHM_NAME "/game_shm_final2"

#define SEM_NAME "/game_sem_final2"



#define MAX_BULLETS 20

#define FPS_DELAY 30000   // 30ms



typedef struct {

    int x, y;

    int active;

    int dir;

} Bullet;



typedef struct {

    pthread_mutex_t lock; // 프로세스 간 공유를 위한 뮤텍스

    int p1_x, p1_y;

    int p2_x, p2_y;



    Bullet bullets[MAX_BULLETS];



    int game_over;

} GameState;



GameState *state;

int player_id;

WINDOW *win;



// 🔫 입력 스레드

void* input_thread(void* arg) {

    int ch;



    while (1) {

        ch = getch();

        

        if (ch == ERR) {

            usleep(1000); // 입력이 없을 때 CPU 부하 감소

            continue;

        }

        if (state->game_over) break;



        pthread_mutex_lock(&state->lock);



        int *px = (player_id == 1) ? &state->p1_x : &state->p2_x;

        int *py = (player_id == 1) ? &state->p1_y : &state->p2_y;



        if (ch == KEY_LEFT)  (*px)--;

        if (ch == KEY_RIGHT) (*px)++;

        if (ch == KEY_UP)    (*py)--;

        if (ch == KEY_DOWN)  (*py)++;



        // 총알 발사

        if (ch == ' ') {

            for (int i = 0; i < MAX_BULLETS; i++) {

                if (!state->bullets[i].active) {

                    state->bullets[i].active = 1;
                    state->bullets[i].x = *px;
                    state->bullets[i].y = *py;
                    state->bullets[i].dir = (player_id == 1) ? -1 : +1;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&state->lock);
    }

    return NULL;
}

// 🎯 게임 업데이트 (player1만)
void update_game() {
    static int tick = 0;
    tick++;
    if (tick % 2 != 0) return; // 속도 조절

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!state->bullets[i].active) continue;
        state->bullets[i].y += state->bullets[i].dir;

        // 충돌 체크
        if (state->bullets[i].x == state->p2_x &&
            state->bullets[i].y == state->p2_y) {
            state->game_over = 1;
        }

        if (state->bullets[i].x == state->p1_x &&
            state->bullets[i].y == state->p1_y) {
            state->game_over = 1;
        }

        // 화면 밖
        if (state->bullets[i].y < 0 || state->bullets[i].y >= LINES) {
            state->bullets[i].active = 0;
        }
    }
}

// 🎨 렌더
void render() {
    werase(win);
    mvwaddch(win, state->p1_y, state->p1_x, 'A');
    mvwaddch(win, state->p2_y, state->p2_x, 'B');

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (state->bullets[i].active) {
            mvwaddch(win,
                state->bullets[i].y,
                state->bullets[i].x,
                '*');
        }
    }

    if (state->game_over) {
        mvwprintw(win, LINES/2, COLS/2-5, "GAME OVER");
    }

    wrefresh(win);
}

int main() {
    int fd;
    int is_creator = 0;

    // shm 생성
    fd = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (fd >= 0) {
        is_creator = 1;
        ftruncate(fd, sizeof(GameState));
    } else {
        fd = shm_open(SHM_NAME, O_RDWR, 0666);
    }
    state = mmap(NULL, sizeof(GameState),
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED, fd, 0);
    if (is_creator) {
        // 뮤텍스 속성 설정: 프로세스 간 공유 가능하도록
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        memset(state, 0, sizeof(GameState));
        pthread_mutex_init(&state->lock, &attr);
        state->p1_x = 10; state->p1_y = 10;
        state->p2_x = 40; state->p2_y = 10;
        player_id = 1;
    } else {
        player_id = 2;
    }

    // ncurses
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    win = newwin(LINES, COLS, 0, 0);
    pthread_t tid;
    pthread_create(&tid, NULL, input_thread, NULL);

    // 🔥 게임 루프 (핵심)

    while (1) {
        pthread_mutex_lock(&state->lock);
        if (player_id == 1) {
            update_game();
        }
        render();
        pthread_mutex_unlock(&state->lock);
        usleep(FPS_DELAY);

        if (state->game_over) break;
    }

    if (is_creator) {
        shm_unlink(SHM_NAME); // 게임 종료 시 생성자가 제거
    }
    endwin();

    return 0;
}