/*
*  display.h:
*  화면에 게임 정보를 출력
*  맵, 커서, 시스템 메시지, 정보창, 자원 상태 등등
*  io.c에 있는 함수들을 사용함
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "common.h"

// 표시할 색상 정의
#define COLOR_DEFAULT 15
#define COLOR_CURSOR 112
#define COLOR_RESOURCE 14
#define COLOR_BLUE 9
#define COLOR_RED 12
#define COLOR_YELLOW 14
#define COLOR_BROWN 6
#define COLOR_BLACK 0
#define COLOR_GRAY 8

// 지금은 자원, 맵, 커서만 표시
// 앞으로 화면에 표시할 내용들 여기에 추가하기
void display(
    RESOURCE resource,
    char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
    CURSOR cursor
);
void clear_status(void);

// 시스템 메시지 및 명령창 출력
void display_system_message(const char* message); // 시스템 메시지 출력
void display_command(); // 명령창에 텍스트 표시
void clear_command(void);                        // 명령창 초기화

// 메시지 관련
#define MAX_MESSAGES 5 // 최대 메시지 수

#endif