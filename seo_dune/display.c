#include "display.h"
#include "io.h"

// �ܺο��� ����� cursor ������ ����
extern CURSOR cursor;

// ����� ������� �»��(topleft) ����
const POSITION resource_pos = { 0, 0 };
const POSITION map_pos = { 1, 0 };
const POSITION status_pos = { 1, MAP_WIDTH + 4 };
const POSITION command_pos = { MAP_HEIGHT + 2, MAP_WIDTH + 4 };
const POSITION system_msg_pos = { MAP_HEIGHT + 2, 0 };

char backbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };
char frontbuf[MAP_HEIGHT][MAP_WIDTH] = { 0 };

CURSOR selected_cursor = { { -1, -1 }, { -1, -1 } };  // ���õ� ��ü ��ġ�� ����

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

                // �� ��ȣ�� �´� ���� ����
                if (ch == 'B') {
                    // ���ϴ� B�� �Ķ���, ���� B�� �ٸ� �������� ����
                    if (i >= MAP_HEIGHT / 2) {
                        color = COLOR_BLUE;  // ���ϴ� ����
                    }
                    else {
                        color = COLOR_RED;  // ���� ����
                    }
                }
                else if (ch == 'H') {
                    if (i >= MAP_HEIGHT / 2) {
                        color = COLOR_BLUE;  // ���ϴ� �Ϻ�����
                    }
                    else {
                        color = COLOR_RED;  // ���� �Ϻ�����
                    }
                }
                else if (ch == 'W') {
                    color = COLOR_YELLOW;  // �����
                }
                else if (ch == 'S') {
                    color = COLOR_BROWN;  // �����̽�
                }
                else if (ch == 'R') {
                    color = COLOR_GRAY;  // ����
                }

                set_color(color);
                printc(padd(map_pos, pos), ch, color);
            }
            frontbuf[i][j] = backbuf[i][j];
        }
    }
}

// Ŀ�� �̵� �� ���� ��ġ�� ���� ��ġ�� �����ϸ� ǥ��
void display_cursor(CURSOR cursor) {
    static POSITION prev_positions[10];
    static int prev_index = 0;

    // ���� Ŀ�� ��ġ�� ��� ������ �´� �������� ����
    for (int i = 0; i < prev_index; i++) {
        POSITION pos = prev_positions[i];
        char ch = frontbuf[pos.row][pos.column];

        // ���ڿ� �´� ���� ����
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

        // ���� ������ �������� ����
        printc(padd(map_pos, pos), ch, color);
    }
    prev_index = 0; // ��� ������ ���� �� �ε��� �ʱ�ȭ

    // ���� ��ġ�� ���ο� Ŀ�� ��ġ�� ǥ��
    prev_positions[prev_index++] = cursor.current;
    char curr_ch = frontbuf[cursor.current.row][cursor.current.column];
    printc(padd(map_pos, cursor.current), curr_ch, COLOR_CURSOR);
}

void display_status(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(status_pos);

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
            printf("����â: �Ӽ� - �α� �ִ�ġ ����\n");
            break;
        case 'G':
            printf("����â: â�� - �����̽� ���� �ִ�ġ ����\n");
            break;
        case 'W':
            printf("����â: ����� - ���� �Ұ�, ū ���� ����\n");
            break;
        case 'H':
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
    gotoxy(command_pos);
    if (selected_cursor.current.row != -1 && selected_cursor.current.column != -1) {
        printf("����â: ���õ� ����Ƽ/�ǹ��� ���� �� �ִ� ���ε�\n");
    }
    else {
        printf("����â: ���õ� ����Ƽ/�ǹ��� ���� �� �ִ� ���ε�\n");
    }
}

void display_system_message(void) {
    set_color(COLOR_DEFAULT);
    gotoxy(system_msg_pos);
    printf("�ý��� �޽��� â: �ý��� ��� �� ����\n");
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
