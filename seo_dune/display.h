/*
*  display.h:
*  ȭ�鿡 ���� ������ ���
*  ��, Ŀ��, �ý��� �޽���, ����â, �ڿ� ���� ���
*  io.c�� �ִ� �Լ����� �����
*/

#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "common.h"

// ǥ���� ���� ����
#define COLOR_DEFAULT 15
#define COLOR_CURSOR 112
#define COLOR_RESOURCE 14
#define COLOR_BLUE 9
#define COLOR_RED 12
#define COLOR_YELLOW 14
#define COLOR_BROWN 6
#define COLOR_BLACK 0
#define COLOR_GRAY 8

// ������ �ڿ�, ��, Ŀ���� ǥ��
// ������ ȭ�鿡 ǥ���� ����� ���⿡ �߰��ϱ�
void display(
    RESOURCE resource,
    char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH],
    CURSOR cursor
);
void clear_status(void);

// �ý��� �޽��� �� ���â ���
void display_system_message(const char* message); // �ý��� �޽��� ���
void display_command(); // ���â�� �ؽ�Ʈ ǥ��
void clear_command(void);                        // ���â �ʱ�ȭ

// �޽��� ����
#define MAX_MESSAGES 5 // �ִ� �޽��� ��

#endif