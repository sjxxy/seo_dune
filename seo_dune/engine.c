#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"
//TEST
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);

/* ================= control =================== */
int sys_clock = 0;  // system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };

/* ================= game data =================== */
char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {  // [MODIFIED]
    .spice = 10,        // [MODIFIED]
    .spice_max = 100,   // [MODIFIED]
    .population = 5,    // [MODIFIED]
    .population_max = 50 // [MODIFIED]
};

OBJECT_SAMPLE obj = {
    .pos = {1, 1},
    .dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
    .repr = 'o',
    .speed = 300,
    .next_move_time = 300
};

/* ================= main() =================== */
int main(void) {
    srand((unsigned int)time(NULL));

    init();
    intro();
    display(resource, map, cursor);  // [MODIFIED] Added arguments to match updated display() function signature

    while (1) {
        // loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
        KEY key = get_key();

        // Ű �Է��� ������ ó��
        if (is_arrow_key(key)) {
            cursor_move(ktod(key));
        }
        else {
            // ����Ű ���� �Է�
            switch (key) {
            case k_quit: outro();
            case k_none:
            case k_undef:
            default: break;
            }
        }

        // ���� ������Ʈ ����
        sample_obj_move();

        // ȭ�� ���
        display(resource, map, cursor);  // [MODIFIED] Added arguments to match updated display() function signature
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
    // �����
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
    map[1][12][40] = 'W';

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
}

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
void cursor_move(DIRECTION dir) {
    POSITION curr = cursor.current;
    POSITION new_pos = pmove(curr, dir);

    // validation check
    if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 && \
        1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {

        cursor.previous = cursor.current;
        cursor.current = new_pos;
    }
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
    // ���� ��ġ�� �������� ���ؼ� �̵� ���� ����    
    POSITION diff = psub(obj.dest, obj.pos);
    DIRECTION dir;

    // ������ ����. ������ �ܼ��� ���� �ڸ��� �պ�
    if (diff.row == 0 && diff.column == 0) {
        if (obj.dest.row == 1 && obj.dest.column == 1) {
            // topleft --> bottomright�� ������ ����
            POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
            obj.dest = new_dest;
        }
        else {
            // bottomright --> topleft�� ������ ����
            POSITION new_dest = { 1, 1 };
            obj.dest = new_dest;
        }
        return obj.pos;
    }

    // ������, ������ �Ÿ��� ���ؼ� �� �� �� ������ �̵�
    if (abs(diff.row) >= abs(diff.column)) {
        dir = (diff.row >= 0) ? d_down : d_up;
    }
    else {
        dir = (diff.column >= 0) ? d_right : d_left;
    }

    // validation check
    // next_pos�� ���� ����� �ʰ�, (������ ������)��ֹ��� �ε����� ������ ���� ��ġ�� �̵�
    POSITION next_pos = pmove(obj.pos, dir);
    if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
        1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2 && \
        map[1][next_pos.row][next_pos.column] < 0) {

        return next_pos;
    }
    else {
        return obj.pos;  // ���ڸ�
    }
}

void sample_obj_move(void) {
    if (sys_clock <= obj.next_move_time) {
        // ���� �ð��� �� ����
        return;
    }

    // ������Ʈ(�ǹ�, ���� ��)�� layer1(map[1])�� ����
    map[1][obj.pos.row][obj.pos.column] = -1;
    obj.pos = sample_obj_next_position();
    map[1][obj.pos.row][obj.pos.column] = obj.repr;

    obj.next_move_time = sys_clock + obj.speed;
}
