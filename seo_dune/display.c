#include "display.h"
#include "io.h"

// 출력할 내용들의 좌상단(topleft) 좌표
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION status_pos = { 1, MAP_WIDTH + 4 };
const POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 4 };
const POSITION system_msg_pos = { MAP_HEIGHT + 2, 0 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_status(void);
void display_command(void);
void display_system_message(void);

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
                    color = COLOR_YELLOW;  // 샌드웜
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

// frontbuf[][]에서 커서 위치의 문자를 색만 바꿔서 그대로 다시 출력
void display_cursor(CURSOR cursor) {
    POSITION prev = cursor.previous;
    POSITION curr = cursor.current;

    char ch = frontbuf[prev.row][prev.column];
    printc(padd(map_pos, prev), ch, COLOR_DEFAULT);

    ch = frontbuf[curr.row][curr.column];
    printc(padd(map_pos, curr), ch, COLOR_CURSOR);
}

void display_status(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(status_pos);
    printf("상태창: 선택된 유닛/건물 정보\n");
    // Placeholder for actual status information
}

void display_command(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(command_pos);
    printf("명령창: 선택된 유닛/건물에 내릴 수 있는 명령어\n");
    // Placeholder for actual command information
}

void display_system_message(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(system_msg_pos);
    printf("시스템 메시지 창: 시스템 경고 및 정보\n");
    // Placeholder for actual system messages
}
