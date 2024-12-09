#include "display.h"
#include "io.h"

// 외부에서 선언된 cursor 변수를 참조
extern CURSOR cursor;
int a = 0;
////테스트임
// 출력할 내용들의 좌상단(topleft) 자편
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION status_pos = { 1, MAP_WIDTH + 4 };
const POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 4 };
const POSITION system_msg_pos = { MAP_HEIGHT + 2, 0 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };


CURSOR selected_cursor = { { -1, -1 }, { -1, -1 } };  // 선택된 객체 위치를 저장

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_status(void);
void display_command(void);
void display_system_message(void);
void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void handle_cancel(void);

void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor) {
    display_resource(resource);
    display_map(map);
    display_cursor(cursor);
    display_status();
    display_command();
    display_system_message();
    // Additional display functions can be added here
}

void display_resource(RESOURCE resource) {
    gotoxy(resource_pos);
    printf("Resource Status: Spice = %d/%d, Population = %d/%d\n",
        resource.spice, resource.spice_max,
        resource.population, resource.population_max);
}

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]) {
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            for (int k = 0; k < N_LAYER; k++) {
                if (src[k][i][j] >= 0) {
                    dest[i][j] = src[k][i][j];
                }
            }
        }
    }
}

void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
    project(map, backbuf);

    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (frontbuf[i][j] != backbuf[i][j]) {
                POSITION pos = { i, j };
                char ch = backbuf[i][j];
                int color = COLOR_DEFAULT;

                // 각 기호에 맞는 색상 설정
                if (ch == 'B') {
                    // 좌하단 B는 파란색, 우상단 B는 다른 색상으로 설정
                    if (i >= MAP_HEIGHT / 2) {
                        color = COLOR_BLUE;  // 좌하단 본진
                    }
                    else {
                        color = COLOR_RED;  // 우상단 본진
                    }
                }
                else if (ch == 'H') {
                    if (i >= MAP_HEIGHT / 2) {
                        color = COLOR_BLUE;  // 좌하단 하베스터
                    }
                    else {
                        color = COLOR_RED;  // 우상단 하베스터
                    }
                }
                else if (ch == 'W') {
                    color = COLOR_YELLOW;  // 산드웬
                }
                else if (ch == 'S') {
                    color = COLOR_BROWN;  // 스파이스
                }
                else if (ch == 'R') {
                    color = COLOR_GRAY;  // 바위
                }

                set_color(color);
                printc(padd(map_pos, pos), ch, color);
            }
            frontbuf[i][j] = backbuf[i][j];
        }
    }
}

// 커서 이동 시 이전 위치와 현재 위치를 관리하며 표시
void display_cursor(CURSOR cursor) {
    static POSITION prev_positions[10];
    static int prev_index = 0;

    // 이전 커서 위치를 모두 지형에 맞는 색상으로 복구
    for (int i = 0; i < prev_index; i++) {
        POSITION pos = prev_positions[i];
        char ch = frontbuf[pos.row][pos.column];

        // 문자에 맞는 색상 설정
        int color = COLOR_DEFAULT;
        if (ch == 'B') {
            color = (pos.row >= MAP_HEIGHT / 2) ? COLOR_BLUE : COLOR_RED;
        }
        else if (ch == 'H') {
            color = (pos.row >= MAP_HEIGHT / 2) ? COLOR_BLUE : COLOR_RED;
        }
        else if (ch == 'W') {
            color = COLOR_YELLOW;
        }
        else if (ch == 'S') {
            color = COLOR_BROWN;
        }
        else if (ch == 'R') {
            color = COLOR_GRAY;
        }

        // 원래 문자의 색상으로 복구
        printc(padd(map_pos, pos), ch, color);
    }
    prev_index = 0; // 모든 복구가 끝난 후 인덱스 초기화

    // 현재 위치를 새로운 커서 위치로 표시
    prev_positions[prev_index++] = cursor.current;
    char curr_ch = frontbuf[cursor.current.row][cursor.current.column];
    printc(padd(map_pos, cursor.current), curr_ch, COLOR_CURSOR);
}

void display_status(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(status_pos);

    // 선택된 커서 위치의 오브젝트 정보를 가져오기
    char selected_tile = frontbuf[selected_cursor.current.row][selected_cursor.current.column];

    // 오브젝트에 따른 상태창 메시지 및 명령어 설정
    switch (selected_tile) {
        case ' ':
            printf("상태창: 사막 - 기본지형 (빈칸)\n");
            break;
        case 'P':
            printf("상태창: 장판 - 건물 건설 가능\n");
            break;
        case 'R':
            printf("상태창: 바위 - 샌드웜 통과 불가\n");
            break;
        case 'B':
            printf("상태창: 본진 - 하베스터 생산 가능 (명령어: H)\n");
            break;
        case 'D':
            printf("상태창: 속소 - 인구 최대치 증가\n");
            break;
        case 'G':
            printf("상태창: 창고 - 스파이스 보관 최대치 증가\n");
            break;
        case 'W':
            printf("상태창: 샌드웜 - 공격 불가, 큰 피해 가능\n");
            break;
        case 'H':
            printf("상태창: 하베스터 - 명령어: H (수확), M (이동)\n");
            break;
        case 'S':
            printf("상태창: 병영 - 보병 생산 가능 (명령어: S)\n");
            break;
        case 'F':
            printf("상태창: 공장 - 중전차 생산 가능 (명령어: T)\n");
            break;
        case 'A':
            printf("상태창: 투기장 - 투사 생산 가능 (명령어: F)\n");
            break;
        default:
            printf("상태창: 알 수 없는 오브젝트\n");
            break;
    }
}

void display_command(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(command_pos);
    if (selected_cursor.current.row != -1 && selected_cursor.current.column != -1) {
        printf("메인창: 선택된 유니티/건물에 내릴 수 있는 메인드\n");
    }
    else {
        printf("메인창: 선택된 유니티/건물에 내릴 수 있는 메인드\n");
    }
}

void display_system_message(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(system_msg_pos);
    printf("시스템 메시지 창: 시스템 경고 및 정보\n");
    // Placeholder for actual system messages
}

void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
    char selected_char = map[0][cursor.current.row][cursor.current.column];
    if (selected_char != ' ' && selected_char != '#') {
        selected_cursor.previous = selected_cursor.current;
        selected_cursor.current = cursor.current;
    }
}

void handle_cancel(void) {
    selected_cursor.previous = selected_cursor.current;
    selected_cursor.current.row = -1;
    selected_cursor.current.column = -1;
}
