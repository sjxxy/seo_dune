#include "display.h"
#include "io.h"

// 외부에서 선언된 cursor 변수를 참조
extern CURSOR cursor;
extern RESOURCE resource;
extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
extern int build_timer;

// 출력할 내용들의 좌상단(topleft) 좌표
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION status_pos = { 1, MAP_WIDTH + 4 };
const POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 4 };
const POSITION system_msg_pos = { MAP_HEIGHT + 2, 0 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

CURSOR selected_cursor = { { -1, -1 }, { -1, -1 } }; // 선택된 객체 위치 저장

void project(char src[N_LAYER][MAP_HEIGHT][MAP_WIDTH], char dest[MAP_HEIGHT][MAP_WIDTH]);
void display_resource(RESOURCE resource);
void display_map(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void display_cursor(CURSOR cursor);
void display_status(void);
void display_command(void);
void display_system_message(const char* message);
void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void handle_cancel(void);
void clear_status(void);

static char system_messages[MAX_MESSAGES][40] = { "" }; // 시스템 메시지 저장 배열
static int message_count = 0; // 현재 시스템 메시지 개수

extern int sys_clock;
/// <summary>
/// //
/// </summary>
/// <param name="resource"></param>
/// <param name="map"></param>
/// <param name="cursor"></param>
void display(RESOURCE resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR cursor) {
    display_resource(resource);
    display_map(map);
    display_cursor(cursor);
    display_status();
    display_command(); // 매개변수 없이 호출
    int b = 0;
    // 시스템 메시지는 업데이트 시 호출 필요
    display_system_message(NULL);
}

// 유닛 목록 출력 함수
void display_units(UNIT_INSTANCE** units, int count) {
    printf("=== Unit List ===\n");
    for (int i = 0; i < count; i++) {
        UNIT_INSTANCE* unit = units[i];
        printf("Unit %d: %s at (%d, %d) - Health: %d/%d\n",
            i + 1,
            unit->unit_info.name,
            unit->position.row,
            unit->position.column,
            unit->current_health,
            unit->unit_info.health);
    }
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

                if (ch == 'B') {
                    color = (i >= MAP_HEIGHT / 2) ? COLOR_BLUE : COLOR_RED;
                }
                else if (ch == 'H') {
                    color = (i >= MAP_HEIGHT / 2) ? COLOR_BLUE : COLOR_RED;
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

                set_color(color);
                printc(padd(map_pos, pos), ch, color);
            }
            frontbuf[i][j] = backbuf[i][j];
        }
    }
}

extern int cursor_size; // 전역 변수 cursor_size 선언

void display_cursor(CURSOR cursor) {
    static POSITION prev_positions[100]; // 이전 커서 위치 저장 (최대 100칸)
    static int prev_index = 0;


    if (is_building && (sys_clock / 250) % 2 == 0) { // 250ms마다 상태 반전
        return; // 깜박임 중일 때 커서를 표시하지 않음
    }

    // 이전 커서 위치 지우기
    for (int i = 0; i < prev_index; i++) {
        POSITION pos = prev_positions[i];
        char ch = frontbuf[pos.row][pos.column]; // 원래 문자 가져오기
        int color = COLOR_DEFAULT; // 기본 색상

        // 원래 위치의 색상 복구
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

        // 원래 문자 출력
        printc(padd(map_pos, pos), ch, color);
    }

    // 이전 위치 초기화
    prev_index = 0;

    // 현재 커서 크기만큼 새로운 커서 표시
    for (int i = 0; i < cursor_size; i++) {
        for (int j = 0; j < cursor_size; j++) {
            POSITION curr_pos = { cursor.current.row + i, cursor.current.column + j };

            // 하단 및 오른쪽 경계 체크 추가
            if (curr_pos.row < MAP_HEIGHT - 1 && curr_pos.column < MAP_WIDTH - 1) {
                prev_positions[prev_index++] = curr_pos;

                // 기존 문자 출력
                char curr_ch = frontbuf[curr_pos.row][curr_pos.column];
                printc(padd(map_pos, curr_pos), curr_ch, COLOR_CURSOR);
            }
        }
    }
}


void display_status(void) {
    set_color(COLOR_DEFAULT);

    // 상태창 초기화
    clear_status();
    gotoxy(status_pos);

    // ESC 키 입력 시 "상태창: "만 출력하고 나머지는 공백으로 처리
    if (selected_cursor.current.row == -1 && selected_cursor.current.column == -1) {
        printf("상태창: ");  // "상태창:"은 항상 출력
        return;
    }

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
        printf("상태창: 숙소 - 인구 최대치 증가\n");
        break;
    case 'G':
        printf("상태창: 창고 - 스파이스 보관 최대치 증가\n");
        break;
    case 'W':
        printf("상태창: 샌드웜 - 공격 불가, 큰 피해 가능\n");
        break;
    case 'H':
        /* printf("상태창: 하베스터 - 명령어: H (수확), M (이동)\n");*/
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

    // 명령창 위치로 이동 및 초기화
    gotoxy(command_pos);
    printf("명령창:                                         "); // 명령창 초기화
    gotoxy(command_pos); // 초기화 후 다시 위치 이동

    // 건설 모드 상태 처리
    if (is_building) {
        if (selected_building == '\0') {
            // 건설 모드: 건물 선택 대기 상태
            printf("명령창: P (장판), D (숙소), G (창고)");
        }
        else {
            // 건물 선택 후 건설 대기 상태
            char* building_name = (selected_building == 'P') ? "장판" :
                (selected_building == 'D') ? "숙소" : "창고";
            printf("명령창: %s 선택됨. 스페이스바로 건설 시작.", building_name);
        }
        return;
    }

    // 선택된 오브젝트가 없는 경우
    if (selected_cursor.current.row == -1 && selected_cursor.current.column == -1) {
        printf("명령창: (선택된 오브젝트 없음)");
        return;
    }

    // 선택된 타일에 따른 명령 출력
    char selected_tile = frontbuf[selected_cursor.current.row][selected_cursor.current.column];
    switch (selected_tile) {
    case 'B':
        printf("명령창: H (하베스터 생산)");
        break;
    case 'H':
        printf("명령창: H (수확), M (이동)");
        break;
    case 'S':
        printf("명령창: (스파이스 매장지 - 명령 없음)");
        break;
    case 'W':
        printf("명령창: (샌드웜 - 명령 불가)");
        break;
    case 'P':
        printf("명령창: B (건물 건설)");
        break;
    default:
        printf("명령창: (명령 없음)");
        break;
    }
}






void display_system_message(const char* message) {
    // 시스템 메시지 제목 출력
    POSITION msg_pos = system_msg_pos; // 시스템 메시지 위치
    gotoxy(msg_pos);
    printf("시스템 메시지:"); // 제목 출력

    // 메시지가 NULL일 경우 기존 메시지를 유지
    if (message == NULL) {
        return;
    }

    // 기존 메시지 배열에 새 메시지 추가
    if (message_count < MAX_MESSAGES) {
        strncpy_s(system_messages[message_count], sizeof(system_messages[message_count]), message, 39);
        system_messages[message_count][39] = '\0';
        message_count++;
    }
    else {
        // 메시지가 가득 찼을 경우, 배열을 한 칸씩 위로 밀기
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strncpy_s(system_messages[i - 1], sizeof(system_messages[i - 1]), system_messages[i], 39);
        }
        strncpy_s(system_messages[MAX_MESSAGES - 1], sizeof(system_messages[MAX_MESSAGES - 1]), message, 39);
        system_messages[MAX_MESSAGES - 1][39] = '\0';
    }

    // 시스템 메시지 내용 출력
    msg_pos.row++;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        gotoxy(msg_pos);
        if (i < message_count) {
            printf("%-40s", system_messages[i]); // 메시지 출력
        }
        else {
            printf("%-40s", ""); // 빈 줄 출력
        }
        msg_pos.row++;
    }
}



void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
    char selected_char = map[0][cursor.current.row][cursor.current.column];
    if (selected_char != ' ' && selected_char != '#') {
        selected_cursor.previous = selected_cursor.current;
        selected_cursor.current = cursor.current;
    }
}

void handle_cancel(void) {
    if (is_building) {
        cancel_building(); // 내부적으로 is_building = false; 실행
        display_system_message("건설이 취소되었습니다."); // 시스템 메시지 출력
        display(resource, map, cursor); // 화면 갱신
    }
}



void clear_status(void) {
    set_color(COLOR_DEFAULT);

    // 상태창 첫 줄 비우기
    gotoxy(status_pos);
    printf("                                              \r");

    // 상태창 두 번째 줄 비우기
    POSITION second_line_pos = { status_pos.row + 1, status_pos.column };
    gotoxy(second_line_pos);
    printf("                                              \r");
}
