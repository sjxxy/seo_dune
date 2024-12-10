#include "display.h"
#include "io.h"

// �ܺο��� ����� cursor ������ ����
extern CURSOR cursor;
extern RESOURCE resource;
extern char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH];
extern int build_timer;

// ����� ������� �»��(topleft) ��ǥ
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION status_pos = { 1, MAP_WIDTH + 4 };
const POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 4 };
const POSITION system_msg_pos = { MAP_HEIGHT + 2, 0 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

CURSOR selected_cursor = { { -1, -1 }, { -1, -1 } }; // ���õ� ��ü ��ġ ����

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

static char system_messages[MAX_MESSAGES][40] = { "" }; // �ý��� �޽��� ���� �迭
static int message_count = 0; // ���� �ý��� �޽��� ����

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
    display_command(); // �Ű����� ���� ȣ��
    int b = 0;
    // �ý��� �޽����� ������Ʈ �� ȣ�� �ʿ�
    display_system_message(NULL);
}

// ���� ��� ��� �Լ�
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

extern int cursor_size; // ���� ���� cursor_size ����

void display_cursor(CURSOR cursor) {
    static POSITION prev_positions[100]; // ���� Ŀ�� ��ġ ���� (�ִ� 100ĭ)
    static int prev_index = 0;


    if (is_building && (sys_clock / 250) % 2 == 0) { // 250ms���� ���� ����
        return; // ������ ���� �� Ŀ���� ǥ������ ����
    }

    // ���� Ŀ�� ��ġ �����
    for (int i = 0; i < prev_index; i++) {
        POSITION pos = prev_positions[i];
        char ch = frontbuf[pos.row][pos.column]; // ���� ���� ��������
        int color = COLOR_DEFAULT; // �⺻ ����

        // ���� ��ġ�� ���� ����
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

        // ���� ���� ���
        printc(padd(map_pos, pos), ch, color);
    }

    // ���� ��ġ �ʱ�ȭ
    prev_index = 0;

    // ���� Ŀ�� ũ�⸸ŭ ���ο� Ŀ�� ǥ��
    for (int i = 0; i < cursor_size; i++) {
        for (int j = 0; j < cursor_size; j++) {
            POSITION curr_pos = { cursor.current.row + i, cursor.current.column + j };

            // �ϴ� �� ������ ��� üũ �߰�
            if (curr_pos.row < MAP_HEIGHT - 1 && curr_pos.column < MAP_WIDTH - 1) {
                prev_positions[prev_index++] = curr_pos;

                // ���� ���� ���
                char curr_ch = frontbuf[curr_pos.row][curr_pos.column];
                printc(padd(map_pos, curr_pos), curr_ch, COLOR_CURSOR);
            }
        }
    }
}


void display_status(void) {
    set_color(COLOR_DEFAULT);

    // ����â �ʱ�ȭ
    clear_status();
    gotoxy(status_pos);

    // ESC Ű �Է� �� "����â: "�� ����ϰ� �������� �������� ó��
    if (selected_cursor.current.row == -1 && selected_cursor.current.column == -1) {
        printf("����â: ");  // "����â:"�� �׻� ���
        return;
    }

    // ���õ� Ŀ�� ��ġ�� ������Ʈ ������ ��������
    char selected_tile = frontbuf[selected_cursor.current.row][selected_cursor.current.column];

    // ������Ʈ�� ���� ����â �޽��� �� ��ɾ� ����
    switch (selected_tile) {
    case ' ':
        printf("����â: �縷 - �⺻���� (��ĭ)\n");
        break;
    case 'P':
        printf("����â: ���� - �ǹ� �Ǽ� ����\n");
        break;
    case 'R':
        printf("����â: ���� - ����� ��� �Ұ�\n");
        break;
    case 'B':
        printf("����â: ���� - �Ϻ����� ���� ���� (��ɾ�: H)\n");
        break;
    case 'D':
        printf("����â: ���� - �α� �ִ�ġ ����\n");
        break;
    case 'G':
        printf("����â: â�� - �����̽� ���� �ִ�ġ ����\n");
        break;
    case 'W':
        printf("����â: ����� - ���� �Ұ�, ū ���� ����\n");
        break;
    case 'H':
        /* printf("����â: �Ϻ����� - ��ɾ�: H (��Ȯ), M (�̵�)\n");*/
        printf("����â: �Ϻ����� - ��ɾ�: H (��Ȯ), M (�̵�)\n");
        break;
    case 'S':
        printf("����â: ���� - ���� ���� ���� (��ɾ�: S)\n");
        break;
    case 'F':
        printf("����â: ���� - ������ ���� ���� (��ɾ�: T)\n");
        break;
    case 'A':
        printf("����â: ������ - ���� ���� ���� (��ɾ�: F)\n");
        break;
    default:
        printf("����â: �� �� ���� ������Ʈ\n");
        break;
    }
}



void display_command(void) {
    set_color(COLOR_DEFAULT);

    // ���â ��ġ�� �̵� �� �ʱ�ȭ
    gotoxy(command_pos);
    printf("���â:                                         "); // ���â �ʱ�ȭ
    gotoxy(command_pos); // �ʱ�ȭ �� �ٽ� ��ġ �̵�

    // �Ǽ� ��� ���� ó��
    if (is_building) {
        if (selected_building == '\0') {
            // �Ǽ� ���: �ǹ� ���� ��� ����
            printf("���â: P (����), D (����), G (â��)");
        }
        else {
            // �ǹ� ���� �� �Ǽ� ��� ����
            char* building_name = (selected_building == 'P') ? "����" :
                (selected_building == 'D') ? "����" : "â��";
            printf("���â: %s ���õ�. �����̽��ٷ� �Ǽ� ����.", building_name);
        }
        return;
    }

    // ���õ� ������Ʈ�� ���� ���
    if (selected_cursor.current.row == -1 && selected_cursor.current.column == -1) {
        printf("���â: (���õ� ������Ʈ ����)");
        return;
    }

    // ���õ� Ÿ�Ͽ� ���� ��� ���
    char selected_tile = frontbuf[selected_cursor.current.row][selected_cursor.current.column];
    switch (selected_tile) {
    case 'B':
        printf("���â: H (�Ϻ����� ����)");
        break;
    case 'H':
        printf("���â: H (��Ȯ), M (�̵�)");
        break;
    case 'S':
        printf("���â: (�����̽� ������ - ��� ����)");
        break;
    case 'W':
        printf("���â: (����� - ��� �Ұ�)");
        break;
    case 'P':
        printf("���â: B (�ǹ� �Ǽ�)");
        break;
    default:
        printf("���â:");
        break;
    }
}






void display_system_message(const char* message) {
    // �ý��� �޽��� ���� ���
    POSITION msg_pos = system_msg_pos; // �ý��� �޽��� ��ġ
    gotoxy(msg_pos);
    printf("�ý��� �޽���:"); // ���� ���

    // �޽����� NULL�� ��� ���� �޽����� ����
    if (message == NULL) {
        return;
    }

    // ���� �޽��� �迭�� �� �޽��� �߰�
    if (message_count < MAX_MESSAGES) {
        strncpy_s(system_messages[message_count], sizeof(system_messages[message_count]), message, 39);
        system_messages[message_count][39] = '\0';
        message_count++;
    }
    else {
        // �޽����� ���� á�� ���, �迭�� �� ĭ�� ���� �б�
        for (int i = 1; i < MAX_MESSAGES; i++) {
            strncpy_s(system_messages[i - 1], sizeof(system_messages[i - 1]), system_messages[i], 39);
        }
        strncpy_s(system_messages[MAX_MESSAGES - 1], sizeof(system_messages[MAX_MESSAGES - 1]), message, 39);
        system_messages[MAX_MESSAGES - 1][39] = '\0';
    }

    // �ý��� �޽��� ���� ���
    msg_pos.row++;
    for (int i = 0; i < MAX_MESSAGES; i++) {
        gotoxy(msg_pos);
        if (i < message_count) {
            printf("%-40s", system_messages[i]); // �޽��� ���
        }
        else {
            printf("%-40s", ""); // �� �� ���
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
        cancel_building(); // ���������� is_building = false; ����
        display_system_message("�Ǽ��� ��ҵǾ����ϴ�."); // �ý��� �޽��� ���
        display(resource, map, cursor); // ȭ�� ����
    }
}



void clear_status(void) {
    set_color(COLOR_DEFAULT);

    // ����â ù �� ����
    gotoxy(status_pos);
    printf("                                              \r");

    // ����â �� ��° �� ����
    POSITION second_line_pos = { status_pos.row + 1, status_pos.column };
    gotoxy(second_line_pos);
    printf("                                              \r");
}
