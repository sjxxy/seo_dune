/*
* raw(?) I/O
*/
#include "io.h"
#include <Windows.h>
#include <conio.h>
#include "common.h"

void gotoxy(POSITION pos) {
    COORD coord = { pos.column, pos.row }; // ��, �� �ݴ�� ����
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printc(POSITION pos, char ch, int color) {
    if (color >= 0) {
        set_color(color);
    }
    gotoxy(pos);
    printf("%c", ch);
}

KEY get_key(void) {
    if (!_kbhit()) {
        return k_none;
    }

    int byte = _getch();
    switch (byte) {
    case 'B': case 'b': return 'B';
    case 'q': return k_quit;
    case 27: return k_esc;
    case 32: return k_space;
    case 'P': case 'p': return 'P';
    case 'D': case 'd': return 'D';
    case 'G': case 'g': return 'G';
    case 'X': case 'x': return 'X';
    case 'H': case 'h': return 'H';
    case 224:
        byte = _getch();
        switch (byte) {
        case 72: return k_up;
        case 75: return k_left;
        case 77: return k_right;
        case 80: return k_down;
        default: return k_undef;
        }
    default: return k_undef;
    }
}