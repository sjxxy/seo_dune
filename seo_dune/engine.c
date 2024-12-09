#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

// 외부에서 선언된 selected_cursor 변수를 참조
extern CURSOR selected_cursor;

// TEST
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void handle_spacebar(void);   // 스페이스바 처리 함수
void handle_esc(void);        // ESC 키 처리 함수

/* ================= control =================== */
int sys_clock = 0;  // system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };

/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {
    .spice = 10,
    .spice_max = 100,
    .population = 5,
    .population_max = 50
};

OBJECT_SAMPLE obj = {
    .pos = {1, 1},
    .dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
    .repr = 'o',
    .speed = 300,
    .next_move_time = 300
};

/* ================= 추가 변수 =================== */
KEY last_key = k_none;        // 마지막 입력된 키를 저장
clock_t last_key_time = 0;    // 마지막 키 입력 시간을 저장
int double_click_delay = 200; // 더블클릭 인식 시간(밀리초 단위) 설정
int multi_move_distance = 5;  // 더블클릭 시 이동할 칸 수 설정 (5칸 이상)

/* ================= main() =================== */
int main(void) {
    srand((unsigned int)time(NULL));

    init();
    intro();
    display(resource, map, cursor);  // [MODIFIED] Added arguments to match updated display() function signature

    while (1) {
        // loop 돌 때마다(즉, TICK==10ms마다) 키 입력 확인
        KEY key = get_key();

        // 방향키 입력이 있으면 처리
        if (is_arrow_key(key)) {
            clock_t current_time = clock();
            int time_diff = (int)((current_time - last_key_time) * 1000 / CLOCKS_PER_SEC);

            // 동일한 키가 짧은 시간 내에 두 번 입력되면 여러 칸 이동
            if (key == last_key && time_diff <= double_click_delay) {
                for (int i = 0; i < multi_move_distance; i++) {
                    cursor_move(ktod(key));  // 여러 칸 이동
                }
            }
            else {
                cursor_move(ktod(key)); // 한 칸 이동
            }

            // 마지막 키와 시간 갱신
            last_key = key;
            last_key_time = current_time;
        }
        else {
            // 방향키 외의 입력 처리
            switch (key) {
            case k_quit:
                outro();
                break;
            case k_esc:
                handle_esc();  // ESC 키 눌렀을 때 처리
                break;
            case k_space:
                handle_spacebar();  // 스페이스바 눌렀을 때 처리
                break;
            case k_none:
            case k_undef:
            default:
                break;
            }
        }

        // 샘플 오브젝트 동작
        sample_obj_move();

        // 화면 출력
        display(resource, map, cursor);  // [MODIFIED] display 함수에 맞게 인자 추가
        Sleep(TICK);
        sys_clock += 10;
    }
}

/* ================= subfunctions =================== */
void intro(void) {
    printf("DUNE 1.5\n");
    Sleep(2000);
    system("cls");
}

void outro(void) {
    printf("exiting...\n");
    exit(0);
}

void init(void) {
    // 초기 상태 설정 [UPDATED WITH BOUNDARY IN MIND]
    // 좌하단 본진(B) 배치 (2x2 형태로 위치 조정)
    map[0][MAP_HEIGHT - 3][1] = 'B';
    map[0][MAP_HEIGHT - 3][2] = 'B';
    map[0][MAP_HEIGHT - 2][1] = 'B';
    map[0][MAP_HEIGHT - 2][2] = 'B';
    // 좌하단 장판(P) 배치
    map[0][MAP_HEIGHT - 3][3] = 'P';
    map[0][MAP_HEIGHT - 2][3] = 'P';
    map[0][MAP_HEIGHT - 3][4] = 'P';
    map[0][MAP_HEIGHT - 2][4] = 'P';
    // 우상단 본진(B) 배치
    map[0][1][MAP_WIDTH - 2] = 'B';
    map[0][2][MAP_WIDTH - 2] = 'B';
    map[0][1][MAP_WIDTH - 3] = 'B';
    map[0][2][MAP_WIDTH - 3] = 'B';
    // 우상단 장판 배치
    map[0][1][MAP_WIDTH - 5] = 'P';
    map[0][2][MAP_WIDTH - 5] = 'P';
    map[0][1][MAP_WIDTH - 4] = 'P';
    map[0][2][MAP_WIDTH - 4] = 'P';
    // 좌하단 하베스터(H) 배치   
    map[1][MAP_HEIGHT - 4][1] = 'H';

    // 우상단 하베스터(H) 배치
    map[1][3][MAP_WIDTH - 2] = 'H';

    // 좌하단 스파이스(5) 배치
    map[0][MAP_HEIGHT - 6][1] = 'S';

    // 우상단 스파이스(5) 배치
    map[0][5][MAP_WIDTH - 2] = 'S';

    // 바위(R) 배치 (맵 내 곳곳에 배치, 테두리 고려)
    map[0][4][53] = 'R';
    map[0][14][55] = 'R';
    map[0][12][6] = 'R';

    map[0][3][25] = 'R';
    map[0][3][26] = 'R';
    map[0][4][25] = 'R';
    map[0][4][26] = 'R';

    map[0][13][28] = 'R';
    map[0][13][29] = 'R';
    map[0][14][28] = 'R';
    map[0][14][29] = 'R';

    // 샌드웜 생성
    map[1][2][10] = 'W';
    map[1][13][45] = 'W';

    // layer 0(map[0])에 지형 생성
    // 외곽 벽 생성
    for (int j = 0; j < MAP_WIDTH; j++) {
        map[0][0][j] = '#';
        map[0][MAP_HEIGHT - 1][j] = '#';
    }
    for (int i = 1; i < MAP_HEIGHT - 1; i++) {
        map[0][i][0] = '#';
        map[0][i][MAP_WIDTH - 1] = '#';
    }

    // 빈 공간 초기화
    for (int i = 1; i < MAP_HEIGHT - 1; i++) {
        for (int j = 1; j < MAP_WIDTH - 1; j++) {
            if (map[0][i][j] == 0) {
                map[0][i][j] = ' ';
            }
        }
    }

    // layer 1(map[1])은 비워 두기(-1로 채움)
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (map[1][i][j] == 0) {
                map[1][i][j] = -1;
            }
        }
    }

    // 초기 자원 설정 [ADDED]
    resource.spice = 10;
    resource.spice_max = 100;
    resource.population = 5;
    resource.population_max = 50;

    // 초기 selected_cursor 위치를 커서 위치로 설정
    selected_cursor.previous = cursor.current;
    selected_cursor.current = cursor.current;
}

// (가능하다면) 지정한 방향으로 커서 이동
void cursor_move(DIRECTION dir) {
    static POSITION prev_positions[10];
    static int prev_index = 0;

    prev_positions[prev_index++] = cursor.current;

    POSITION new_pos = pmove(cursor.current, dir);

    if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 &&
        1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {
        cursor.previous = cursor.current;
        cursor.current = new_pos;
    }

    if (prev_index >= multi_move_distance) {
        prev_index = 0;
    }
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
    POSITION diff = psub(obj.dest, obj.pos);
    DIRECTION dir;

    if (diff.row == 0 && diff.column == 0) {
        if (obj.dest.row == 1 && obj.dest.column == 1) {
            POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
            obj.dest = new_dest;
        }
        else {
            POSITION new_dest = { 1, 1 };
            obj.dest = new_dest;
        }
        return obj.pos;
    }

    if (abs(diff.row) >= abs(diff.column)) {
        dir = (diff.row >= 0) ? d_down : d_up;
    }
    else {
        dir = (diff.column >= 0) ? d_right : d_left;
    }

    POSITION next_pos = pmove(obj.pos, dir);
    if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 &&
        1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 &&
        map[1][next_pos.row][next_pos.column] < 0) {

        return next_pos;
    }
    else {
        return obj.pos;
    }
}

void sample_obj_move(void) {
    if (sys_clock <= obj.next_move_time) {
        return;
    }

    map[1][obj.pos.row][obj.pos.column] = -1;
    obj.pos = sample_obj_next_position();
    map[1][obj.pos.row][obj.pos.column] = obj.repr;

    obj.next_move_time = sys_clock + obj.speed;
}

/* ================= new functions =================== */
void handle_spacebar(void) {
    // 현재 커서 위치를 selected_cursor에 저장
    selected_cursor.previous = selected_cursor.current;
    selected_cursor.current = cursor.current;

    // 선택된 위치의 오브젝트 정보를 업데이트
    display_status();  // 상태창에 선택된 위치의 오브젝트 상태 표시
}

void handle_esc(void) {
    handle_cancel();
}
