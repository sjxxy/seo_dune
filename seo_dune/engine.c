#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

// �ܺο��� ����� selected_cursor ������ ����
extern CURSOR selected_cursor;

// TEST
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void handle_spacebar(void);   // �����̽��� ó�� �Լ�
void handle_esc(void);        // ESC Ű ó�� �Լ�

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

/* ================= �߰� ���� =================== */
KEY last_key = k_none;        // ������ �Էµ� Ű�� ����
clock_t last_key_time = 0;    // ������ Ű �Է� �ð��� ����
int double_click_delay = 200; // ����Ŭ�� �ν� �ð�(�и��� ����) ����
int multi_move_distance = 5;  // ����Ŭ�� �� �̵��� ĭ �� ���� (5ĭ �̻�)

/* ================= main() =================== */
int main(void) {
    srand((unsigned int)time(NULL));

    init();
    intro();
    display(resource, map, cursor);  // [MODIFIED] Added arguments to match updated display() function signature

    while (1) {
        // loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
        KEY key = get_key();

        // ����Ű �Է��� ������ ó��
        if (is_arrow_key(key)) {
            clock_t current_time = clock();
            int time_diff = (int)((current_time - last_key_time) * 1000 / CLOCKS_PER_SEC);

            // ������ Ű�� ª�� �ð� ���� �� �� �ԷµǸ� ���� ĭ �̵�
            if (key == last_key && time_diff <= double_click_delay) {
                for (int i = 0; i < multi_move_distance; i++) {
                    cursor_move(ktod(key));  // ���� ĭ �̵�
                }
            }
            else {
                cursor_move(ktod(key)); // �� ĭ �̵�
            }

            // ������ Ű�� �ð� ����
            last_key = key;
            last_key_time = current_time;
        }
        else {
            // ����Ű ���� �Է� ó��
            switch (key) {
            case k_quit:
                outro();
                break;
            case k_esc:
                handle_esc();  // ESC Ű ������ �� ó��
                break;
            case k_space:
                handle_spacebar();  // �����̽��� ������ �� ó��
                break;
            case k_none:
            case k_undef:
            default:
                break;
            }
        }

        // ���� ������Ʈ ����
        sample_obj_move();

        // ȭ�� ���
        display(resource, map, cursor);  // [MODIFIED] display �Լ��� �°� ���� �߰�
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
    // �ʱ� ���� ���� [UPDATED WITH BOUNDARY IN MIND]
    // ���ϴ� ����(B) ��ġ (2x2 ���·� ��ġ ����)
    map[0][MAP_HEIGHT - 3][1] = 'B';
    map[0][MAP_HEIGHT - 3][2] = 'B';
    map[0][MAP_HEIGHT - 2][1] = 'B';
    map[0][MAP_HEIGHT - 2][2] = 'B';
    // ���ϴ� ����(P) ��ġ
    map[0][MAP_HEIGHT - 3][3] = 'P';
    map[0][MAP_HEIGHT - 2][3] = 'P';
    map[0][MAP_HEIGHT - 3][4] = 'P';
    map[0][MAP_HEIGHT - 2][4] = 'P';
    // ���� ����(B) ��ġ
    map[0][1][MAP_WIDTH - 2] = 'B';
    map[0][2][MAP_WIDTH - 2] = 'B';
    map[0][1][MAP_WIDTH - 3] = 'B';
    map[0][2][MAP_WIDTH - 3] = 'B';
    // ���� ���� ��ġ
    map[0][1][MAP_WIDTH - 5] = 'P';
    map[0][2][MAP_WIDTH - 5] = 'P';
    map[0][1][MAP_WIDTH - 4] = 'P';
    map[0][2][MAP_WIDTH - 4] = 'P';
    // ���ϴ� �Ϻ�����(H) ��ġ   
    map[1][MAP_HEIGHT - 4][1] = 'H';

    // ���� �Ϻ�����(H) ��ġ
    map[1][3][MAP_WIDTH - 2] = 'H';

    // ���ϴ� �����̽�(5) ��ġ
    map[0][MAP_HEIGHT - 6][1] = 'S';

    // ���� �����̽�(5) ��ġ
    map[0][5][MAP_WIDTH - 2] = 'S';

    // ����(R) ��ġ (�� �� ������ ��ġ, �׵θ� ���)
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

    // ����� ����
    map[1][2][10] = 'W';
    map[1][13][45] = 'W';

    // layer 0(map[0])�� ���� ����
    // �ܰ� �� ����
    for (int j = 0; j < MAP_WIDTH; j++) {
        map[0][0][j] = '#';
        map[0][MAP_HEIGHT - 1][j] = '#';
    }
    for (int i = 1; i < MAP_HEIGHT - 1; i++) {
        map[0][i][0] = '#';
        map[0][i][MAP_WIDTH - 1] = '#';
    }

    // �� ���� �ʱ�ȭ
    for (int i = 1; i < MAP_HEIGHT - 1; i++) {
        for (int j = 1; j < MAP_WIDTH - 1; j++) {
            if (map[0][i][j] == 0) {
                map[0][i][j] = ' ';
            }
        }
    }

    // layer 1(map[1])�� ��� �α�(-1�� ä��)
    for (int i = 0; i < MAP_HEIGHT; i++) {
        for (int j = 0; j < MAP_WIDTH; j++) {
            if (map[1][i][j] == 0) {
                map[1][i][j] = -1;
            }
        }
    }

    // �ʱ� �ڿ� ���� [ADDED]
    resource.spice = 10;
    resource.spice_max = 100;
    resource.population = 5;
    resource.population_max = 50;

    // �ʱ� selected_cursor ��ġ�� Ŀ�� ��ġ�� ����
    selected_cursor.previous = cursor.current;
    selected_cursor.current = cursor.current;
}

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
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
    // ���� Ŀ�� ��ġ�� selected_cursor�� ����
    selected_cursor.previous = selected_cursor.current;
    selected_cursor.current = cursor.current;

    // ���õ� ��ġ�� ������Ʈ ������ ������Ʈ
    display_status();  // ����â�� ���õ� ��ġ�� ������Ʈ ���� ǥ��
}

void handle_esc(void) {
    handle_cancel();
}
