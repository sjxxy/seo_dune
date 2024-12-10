#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdbool.h>
#include <Windows.h>
#include <conio.h>
#include <assert.h>

#define TICK 10
#define N_LAYER 2
#define MAP_WIDTH 60
#define MAP_HEIGHT 18
#define MAX_UNITS 100 // supply �ִ밪


extern char frontbuf[MAP_HEIGHT][MAP_WIDTH];

typedef struct {
    int row, column;
} POSITION;

typedef struct {
    POSITION previous;
    POSITION current;
} CURSOR;

typedef enum {
    k_none = 0, k_up, k_right, k_left, k_down,
    k_quit, k_esc, k_space, k_x, // �߰�
    k_undef
} KEY;


typedef enum {
    d_stay = 0, d_up, d_right, d_left, d_down
} DIRECTION;

inline POSITION padd(POSITION p1, POSITION p2) {
    POSITION p = { p1.row + p2.row, p1.column + p2.column };
    return p;
}

inline POSITION psub(POSITION p1, POSITION p2) {
    POSITION p = { p1.row - p2.row, p1.column - p2.column };
    return p;
}

#define is_arrow_key(k) (k_up <= (k) && (k) <= k_down)
#define ktod(k) (DIRECTION)(k)

inline POSITION dtop(DIRECTION d) {
    static POSITION direction_vector[] = { {0, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 0} };
    return direction_vector[d];
}

#define pmove(p, d) (padd((p), dtop(d)))

typedef struct {
    int spice;
    int spice_max;
    int population;
    int population_max;
} RESOURCE;

typedef struct {
    POSITION pos;
    POSITION dest;
    char repr;
    int move_period;
    int next_move_time;
    int speed;
} OBJECT_SAMPLE;

// ����� �Ӽ�
typedef struct {
    POSITION pos;           // ���� ��ġ
    POSITION target;        // ��ǥ ��ġ (���� ����� ����)
    int next_move_time;     // ���� ������ �ð�
    int speed;              // �̵� �ӵ�(ms)
    int next_deposit_time;  // ���� �����̽� �輳 �ð�
    int deposit_period;     // �輳 �ֱ�(ms)
} SANDWORM;


typedef struct {
    SANDWORM sandworms[10];  // �ִ� 10���� ����� ����
    int count;               // ���� ����� ��
} SANDWORM_MANAGER;


typedef struct {
    char name[20];
    int production_cost;
    int population_cost;
    int move_period;
    int attack_power;
    int attack_period;
    int health;
    int vision;
} UNIT;

typedef struct {
    UNIT unit_info;
    POSITION position;
    int current_health;
    int command;  // ��� ����
    POSITION target; // ��� ���
} UNIT_INSTANCE;


void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void handle_cancel(void);

POSITION find_closest_unit(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], POSITION sandworm_pos);
void move_sandworm(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void deposit_spice(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);


void foreach_unit(UNIT_INSTANCE** units, int count, void (*callback)(UNIT_INSTANCE*));
UNIT_INSTANCE* get_unit(UNIT_INSTANCE** units, int count, int index);
void remove_unit(UNIT_INSTANCE** units, int* count, UNIT_INSTANCE* unit_to_remove);

// �ܺο��� ����� �� �ֵ��� extern ����
extern bool is_building;
extern char selected_building;


#endif
