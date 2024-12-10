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
#define MAX_UNITS 100 // supply 최대값


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
    k_quit, k_esc, k_space, k_x, // 추가
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

// 샌드웜 속성
typedef struct {
    POSITION pos;           // 현재 위치
    POSITION target;        // 목표 위치 (가장 가까운 유닛)
    int next_move_time;     // 다음 움직일 시간
    int speed;              // 이동 속도(ms)
    int next_deposit_time;  // 다음 스파이스 배설 시간
    int deposit_period;     // 배설 주기(ms)
} SANDWORM;


typedef struct {
    SANDWORM sandworms[10];  // 최대 10개의 샌드웜 관리
    int count;               // 현재 샌드웜 수
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
    int command;  // 명령 상태
    POSITION target; // 명령 대상
} UNIT_INSTANCE;


void handle_selection(CURSOR cursor, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void handle_cancel(void);

POSITION find_closest_unit(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], POSITION sandworm_pos);
void move_sandworm(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void deposit_spice(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);


void foreach_unit(UNIT_INSTANCE** units, int count, void (*callback)(UNIT_INSTANCE*));
UNIT_INSTANCE* get_unit(UNIT_INSTANCE** units, int count, int index);
void remove_unit(UNIT_INSTANCE** units, int* count, UNIT_INSTANCE* unit_to_remove);

// 외부에서 사용할 수 있도록 extern 선언
extern bool is_building;
extern char selected_building;


#endif
