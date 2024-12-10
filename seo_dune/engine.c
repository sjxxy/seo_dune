#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

// �ܺο��� ����� selected_cursor ������ ����
extern CURSOR selected_cursor;
extern const POSITION map_pos; // �ܺο��� ������ �� �ֵ��� ����

/* ================= �Լ� ���� =================== */
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);
void handle_spacebar(void);
void handle_esc(void);
void create_unit_at_base(RESOURCE* resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR selected_cursor);
void init_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]);
void update_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], RESOURCE* resource);

/* ================= control =================== */
int sys_clock = 0;  // system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, { 1, 1 } };

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

SANDWORM sandworms[10];      // �ִ� 10���� ����� ����
int sandworm_count = 0;      // ���� ����� ��

// ����� �ʱ�ȭ
void init_sandworm(SANDWORM* worm, POSITION start_pos, int speed, int deposit_period) {
	worm->pos = start_pos;
	worm->next_move_time = sys_clock + speed;
	worm->speed = speed;

	// �輳 �ֱ⸦ ����ȭ
	worm->deposit_period = deposit_period; // ������ �κ�
	worm->next_deposit_time = sys_clock + worm->deposit_period;

	worm->target = (POSITION){ -1, -1 }; // �ʱ�ȭ: ��ǥ ����
	/*printf("DEBUG: Initialized Sandworm at (%d, %d) with deposit period = %dms\n",
		worm->pos.row, worm->pos.column, worm->deposit_period);*/
}




// ����� �ʱ�ȭ (���� W ��ġ ���)
void init_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	sandworm_count = 0;
	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			if (map[1][row][col] == 'W' && sandworm_count < 10) {
				init_sandworm(&sandworms[sandworm_count++], (POSITION) { row, col }, 1000, 5000);
			}
		}
	}
}

// ����� ���� ������Ʈ
void update_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], RESOURCE* resource) {
	for (int i = 0; i < sandworm_count; i++) {
		SANDWORM* worm = &sandworms[i];

		// �̵� ó��: ��ǥ�� ��ȿ�� ��� �̵�
		if (worm->target.row == -1 || worm->target.column == -1 ||
			map[1][worm->target.row][worm->target.column] != 'H') {
			worm->target = find_closest_unit(map, worm->pos);

			if (worm->target.row == -1 || worm->target.column == -1) {
				/*printf("DEBUG: No valid target for Sandworm at (%d, %d)\n", worm->pos.row, worm->pos.column);*/
				continue;
			}

			//printf("DEBUG: Sandworm at (%d, %d) updated target to (%d, %d)\n",
			//	worm->pos.row, worm->pos.column, worm->target.row, worm->target.column);
		}

		// �̵�
		move_sandworm(worm, map);

		// �輳 ó�� (�̵� �� �輳)
		if (sys_clock >= worm->next_deposit_time) {
			deposit_spice(worm, map);
		}
	}
}



// ���� ����
bool is_building = false; // �Ǽ� ��� ����
char selected_building = '\0'; // ���õ� �ǹ� ('P', 'B', ��)
int build_timer = 0; // �Ǽ� Ÿ�̸�
int cursor_size = 1; // Ŀ�� ũ�� (1x1 �Ǵ� 2x2)

// �Ǽ� ��� ����
void enter_build_mode() {
	is_building = true; // �Ǽ� ��� Ȱ��ȭ
	selected_building = '\0'; // ���� �ǹ� ���� �� ��
	display_system_message("�Ǽ� ���: P (����), D (����), G (â��)"); // ���â�� �޽��� ǥ��
}



// �ǹ� ����
void select_building(char building_type) {
	selected_building = building_type;
	cursor_size = 2; // Ŀ���� 2x2�� ����
	display_system_message("�ǹ��� �����߽��ϴ�. �����̽��ٷ� �Ǽ� ����.");
}

// �Ǽ� ����
void start_building() {
	if (selected_building == '\0') {
		display_system_message("�Ǽ��� �ǹ��� �����ϼ���!");
		return;
	}
	///asasasasasas
	// �Ǽ� ��ġ Ȯ�� �� Ÿ�̸� ����
	POSITION pos = cursor.current;
	for (int i = 0; i < cursor_size; i++) {
		for (int j = 0; j < cursor_size; j++) {
			POSITION check_pos = { pos.row + i, pos.column + j };
			if (check_pos.row < 1 || check_pos.row >= MAP_HEIGHT - 1 ||
				check_pos.column < 1 || check_pos.column >= MAP_WIDTH - 1) {
				display_system_message("�Ǽ��� ��ġ�� ���� ������ϴ�!");
				return;
			}
			if (map[0][check_pos.row][check_pos.column] != 'P') {
				display_system_message("�Ǽ��� ��ġ�� ������ �����ϴ�!");
				return;
			}
		}
	}

	build_timer = 300; // 3�� ���� �Ǽ�
	char timer_message[40];
	int remaining_time = build_timer * TICK / 1000;
	snprintf(timer_message, sizeof(timer_message), "�Ǽ� ��... ���� �ð�: %d��", remaining_time);
	display_system_message(timer_message); // �ʱ� �޽��� ���

	display(resource, map, cursor); // ȭ�� ����
}



// �Ǽ� ���
void cancel_building() {
	/*printf("DEBUG: cancel_building() ȣ���. �ʱ� ���� - is_building: %d, build_timer: %d\n", is_building, build_timer);*/
	is_building = false;           // �Ǽ� ��� ����
	selected_building = '\0';      // ���õ� �ǹ� �ʱ�ȭ
	build_timer = 0;               // �Ǽ� Ÿ�̸� �ʱ�ȭ
	cursor_size = 1;               // Ŀ���� 1x1�� ����
	/*display_system_message("DEBUG: �Ǽ� ��� �����.");*/
	/*printf("DEBUG: cancel_building() ���� �Ϸ�. ���� ���� - is_building: %d, build_timer: %d\n", is_building, build_timer);*/
}






// �Ǽ� �Ϸ�
void complete_building() {
	POSITION pos = cursor.current;

	// ���õ� ��ġ�� �ǹ� ��ġ
	for (int i = 0; i < cursor_size; i++) {
		for (int j = 0; j < cursor_size; j++) {
			POSITION build_pos = { pos.row + i, pos.column + j };
			map[0][build_pos.row][build_pos.column] = selected_building;
		}
	}

	// ���� �ʱ�ȭ
	is_building = false;
	selected_building = '\0';
	cursor_size = 1; // Ŀ���� ���� ũ��� ����
	display_system_message("�ǹ��� ���������� ��ġ�Ǿ����ϴ�!");
	display(resource, map, cursor); // ȭ�� ����
}






/* ================= �߰� ���� =================== */
KEY last_key = k_none;        // ������ �Էµ� Ű�� ����
clock_t last_key_time = 0;    // ������ Ű �Է� �ð��� ����
int double_click_delay = 200; // ����Ŭ�� �ν� �ð�(�и��� ����)
int multi_move_distance = 5;  // ����Ŭ�� �� �̵��� ĭ ��

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));

	init();
	init_sandworms(map);  // ����� �ʱ�ȭ
	intro();
	display(resource, map, cursor);

	char last_timer_message[40] = "";  // ������ ����� �޽����� ����

	while (1) {
		KEY key = get_key();

		// x Ű�� ���� ó��
		if (key == 'X' || key == 'x') {
			/*printf("DEBUG: x Ű �Էµ�. is_building = %d, build_timer = %d\n", is_building, build_timer);*/
			if (is_building && build_timer > 0) {
				// �Ǽ� ���� �� ���
				/*printf("DEBUG: �Ǽ� ���� �� ��� ���� ����\n");*/
				cancel_building(); // �Ǽ� ���
				display_system_message("�Ǽ� ���� �� ��ҵǾ����ϴ�.");
				display(resource, map, cursor); // ȭ�� ����
				continue; // ���� ������ �̵�
			}
			else if (is_building) {
				// �Ǽ� ��常 ���
				/*printf("DEBUG: �Ǽ� ��� ��� ���� ����\n");*/
				is_building = false;
				selected_building = '\0';
				cursor_size = 1; // Ŀ���� 1x1�� �ǵ���
				display_system_message("�Ǽ� ��尡 ��ҵǾ����ϴ�.");
				display(resource, map, cursor); // ȭ�� ����
				continue; // ���� ������ �̵�
			}
			else {
				/*printf("DEBUG: �Ǽ� ���°� �ƴ�. x Ű �Է� ���õ�\n");*/
				display_system_message("�Ǽ� ���°� �ƴϾ ����� �� �����ϴ�.");
				continue; // ���� ������ �̵�
			}
		}

		// �Ǽ� ���� ����
		if (is_building) {
			if (build_timer > 0) {
				build_timer--; // �Ǽ� Ÿ�̸� ����
				/*printf("DEBUG: build_timer = %d\n", build_timer);*/

				// ���� �ð� ��� �� ǥ��
				char timer_message[40];
				int remaining_time = build_timer * TICK / 1000; // ���� �ð� (�� ����)
				snprintf(timer_message, sizeof(timer_message), "�Ǽ� ��... ���� �ð�: %d��", remaining_time);

				// ���� �޽����� �ٸ� ��쿡�� ���
				if (strcmp(timer_message, last_timer_message) != 0) {
					display_system_message(timer_message);
					strncpy_s(last_timer_message, sizeof(last_timer_message), timer_message, _TRUNCATE);
				}

				// ������ ȿ�� (�Ǽ� ���� ����)
				for (int i = 0; i < cursor_size; i++) {
					for (int j = 0; j < cursor_size; j++) {
						POSITION build_pos = { cursor.current.row + i, cursor.current.column + j };
						if (build_pos.row >= 1 && build_pos.row < MAP_HEIGHT - 1 &&
							build_pos.column >= 1 && build_pos.column < MAP_WIDTH - 1) {
							char ch = frontbuf[build_pos.row][build_pos.column];
							int color = (sys_clock % 500 < 250) ? COLOR_CURSOR : COLOR_DEFAULT; // 500ms �ֱ�
							printc(padd(map_pos, build_pos), ch, color);
						}
					}
				}

				set_color(COLOR_DEFAULT); // ������ �׻� ����

				// �Ǽ� �Ϸ�
				if (build_timer == 0) {
					complete_building();
					is_building = false; // �Ǽ� ��� ����
				}

				// �ð� ��� �� ���
				Sleep(TICK);
				sys_clock += TICK;
				continue; // �Ǽ� ���� ó�� �� ���� ������ �̵�
			}
		}

		// Ű �Է� ó��
		if (is_arrow_key(key)) {
			clock_t current_time = clock();
			int time_diff = (int)((current_time - last_key_time) * 1000 / CLOCKS_PER_SEC);

			if (key == last_key && time_diff <= double_click_delay) {
				// ����Ŭ��: �ָ� �̵�
				for (int i = 0; i < multi_move_distance; i++) {
					cursor_move(ktod(key));
				}
			}
			else {
				// ���� �̵�
				cursor_move(ktod(key));
			}

			last_key = key;
			last_key_time = current_time;
		}
		else {
			// ��Ÿ Ű �Է� ó��
			switch (key) {
			case k_quit:
				outro();
				break;
			case k_esc:
				handle_esc();
				break;

			case k_space:
				if (is_building) {
					if (selected_building == '\0') {
						display_system_message("�Ǽ��� �ǹ��� �����ϼ���!");
					}
					else {
						start_building(); // �Ǽ� ����
					}
				}
				else {
					handle_spacebar(); // �Ϲ� ������ �����̽��� ����
				}
				break;
			case 'B':
			case 'b': // �Ǽ� ��� ����
				if (!is_building) {
					enter_build_mode();
				}
				break;
			case 'H':
			case 'h': // �ҹ��� h�� ó��
				create_unit_at_base(&resource, map, selected_cursor);
				break;
			case 'P':
			case 'p': // �Ǽ��� ���� ����
			case 'D':
			case 'd': // �Ǽ��� ���� ����
			case 'G':
			case 'g': // �Ǽ��� â�� ����
				if (is_building) {
					select_building(key);
				}
				break;
			default:
				break;
			}
		}

		// ���� ������Ʈ ����
		sample_obj_move();

		// ����� ������Ʈ: ��ǥ�� ���� ��� �߰� ó�� ����
		update_sandworms(map, &resource);

		// ���� ȭ�� ����
		display(resource, map, cursor);

		// �߰�: Ŀ���� �� �Ʒ��� �̵� �� ȭ���� ����� ó��
		gotoxy((POSITION) { MAP_HEIGHT + 5, 0 });
		printf("\n"); // ��Ȯ�� �ٹٲ� �߰�

		// �ý��� �ð� ����
		Sleep(TICK);
		sys_clock += TICK;
	}


}




/* ================= subfunctions =================== */
void intro(void) {
	printf("DUNE 1.5\n");
	Sleep(2000);
	system("cls");
}

void outro(void) {
	printf("Exiting...\n");
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


	// �ʱ� �ڿ� ����
	resource.spice = 50;
	resource.spice_max = 100;
	resource.population = 5;
	resource.population_max = 50;

	// �ʱ� selected_cursor ��ġ�� Ŀ�� ��ġ�� ����
	selected_cursor.previous = cursor.current;
	selected_cursor.current = cursor.current;
}


/* ================= cursor_move =================== */
void cursor_move(DIRECTION dir) {
	POSITION new_pos = pmove(cursor.current, dir);

	if (new_pos.row >= 1 && new_pos.row + cursor_size - 1 < MAP_HEIGHT - 1 &&
		new_pos.column >= 1 && new_pos.column + cursor_size - 1 < MAP_WIDTH - 1) {
		cursor.previous = cursor.current;
		cursor.current = new_pos;
	}
}

/* ================= sample_obj_move =================== */
POSITION sample_obj_next_position(void) {
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	if (diff.row == 0 && diff.column == 0) {
		obj.dest.row = (obj.dest.row == 1) ? MAP_HEIGHT - 2 : 1;
		obj.dest.column = (obj.dest.column == 1) ? MAP_WIDTH - 2 : 1;
		return obj.pos;
	}

	dir = (abs(diff.row) >= abs(diff.column))
		? ((diff.row >= 0) ? d_down : d_up)
		: ((diff.column >= 0) ? d_right : d_left);

	POSITION next_pos = pmove(obj.pos, dir);
	if (map[0][next_pos.row][next_pos.column] == ' ') {
		return next_pos;
	}
	return obj.pos;
}
//�� ������
//test

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) return;

	map[1][obj.pos.row][obj.pos.column] = -1; // ���� ��ġ �ʱ�ȭ
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}

// ����ư �Ÿ� ���
int manhattan_distance(POSITION p1, POSITION p2) {
	return abs(p1.row - p2.row) + abs(p1.column - p2.column);
}


// ���� ����� ���� ã��
POSITION find_closest_unit(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], POSITION sandworm_pos) {
	POSITION closest = { -1, -1 }; // �⺻��: ���� ����
	int min_distance = INT_MAX;

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			if (map[1][row][col] == 'H') { // ����(H) �߰�
				// ����ư �Ÿ� ���
				int distance = manhattan_distance(sandworm_pos, (POSITION) { row, col });

				// ��ȿ�� �̵� ������� Ȯ��
				if (map[0][row][col] != 'S' && map[0][row][col] != 'R' && map[0][row][col] != '#' &&
					distance < min_distance) {
					closest.row = row;
					closest.column = col;
					min_distance = distance;
				}
			}
		}
	}

	//// ����� �޽���
	//if (closest.row == -1 && closest.column == -1) {
	//	printf("DEBUG: No harvester found for sandworm at (%d, %d)\n", sandworm_pos.row, sandworm_pos.column);
	//}
	//else {
	//	printf("DEBUG: Closest harvester to sandworm at (%d, %d) is at (%d, %d), distance = %d\n",
	//		sandworm_pos.row, sandworm_pos.column, closest.row, closest.column, min_distance);
	//}

	return closest;
}






// ����� �̵�
void move_sandworm(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	if (sys_clock < worm->next_move_time) return; // �̵� ���� �ð� Ȯ��

	POSITION diff = psub(worm->target, worm->pos);
	DIRECTION dir;

	// ���� ���� (���� �켱)
	if (abs(diff.row) > abs(diff.column)) {
		dir = (diff.row > 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column > 0) ? d_right : d_left;
	}

	POSITION next_pos = pmove(worm->pos, dir);

	// �̵� ���� ���� Ȯ��
	if (next_pos.row > 0 && next_pos.row < MAP_HEIGHT - 1 &&
		next_pos.column > 0 && next_pos.column < MAP_WIDTH - 1 &&
		map[0][next_pos.row][next_pos.column] == ' ') {

		// ���� ��ġ �ʱ�ȭ
		map[1][worm->pos.row][worm->pos.column] = -1;

		// �� ��ġ�� �̵�
		worm->pos = next_pos;
		map[1][worm->pos.row][worm->pos.column] = 'W';

		/*printf("DEBUG: Sandworm moved to (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}
	else {
		/*printf("DEBUG: Movement blocked for Sandworm at (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}

	// ���� �̵� �ð� ����
	worm->next_move_time = sys_clock + worm->speed;
}



















// �����̽� �輳 (�輳 �ֱ� ����ȭ)
void deposit_spice(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	if (sys_clock < worm->next_deposit_time) return;

	// ���� ��ġ�� �� �������� Ȯ��
	if (map[0][worm->pos.row][worm->pos.column] == ' ') {
		map[0][worm->pos.row][worm->pos.column] = 'S'; // �����̽� ��ġ
		/*printf("DEBUG: Sandworm deposited spice at (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}

	worm->next_deposit_time = sys_clock + worm->deposit_period; // ���� �輳 �ð� ����
}



UNIT_INSTANCE* create_unit(UNIT* unit_info, POSITION position) {
	UNIT_INSTANCE* unit = malloc(sizeof(UNIT_INSTANCE));
	if (!unit) {
		printf("ERROR: Memory allocation failed for UNIT_INSTANCE.\n");
		return NULL;
	}
	unit->unit_info = *unit_info;
	unit->position = position;
	unit->current_health = unit_info->health;
	unit->command = 0;
	unit->target = (POSITION){ -1, -1 };
	return unit;
}

void add_unit(UNIT_INSTANCE** units, int* count, UNIT_INSTANCE* new_unit) {
	if (*count >= MAX_UNITS) {
		printf("ERROR: Unit limit reached. Cannot add more units.\n");
		return;
	}
	units[*count] = new_unit;
	(*count)++;
}

void foreach_unit(UNIT_INSTANCE** units, int count, void (*callback)(UNIT_INSTANCE*)) {
	for (int i = 0; i < count; i++) {
		callback(units[i]);
	}
}

UNIT_INSTANCE* get_unit(UNIT_INSTANCE** units, int count, int index) {
	if (index < 0 || index >= count) {
		printf("ERROR: Invalid unit index.\n");
		return NULL;
	}
	return units[index];
}

void remove_unit(UNIT_INSTANCE** units, int* count, UNIT_INSTANCE* unit_to_remove) {
	for (int i = 0; i < *count; i++) {
		if (units[i] == unit_to_remove) {
			free(units[i]);
			for (int j = i; j < *count - 1; j++) {
				units[j] = units[j + 1];
			}
			(*count)--;
			break;
		}
	}
}

// Example callback for foreach_unit
void print_unit_info(UNIT_INSTANCE* unit) {
	printf("Unit: %s at (%d, %d), Health: %d\n",
		unit->unit_info.name,
		unit->position.row,
		unit->position.column,
		unit->current_health);
}











/* ================= ���� ���� �Լ� =================== */
void create_unit_at_base(RESOURCE* resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR selected_cursor) {
	// �������� �Ϻ����͸� ��ġ�� Ÿ���� ã��
	char selected_tile = map[0][selected_cursor.current.row][selected_cursor.current.column];
	if (selected_tile != 'B') {
		display_system_message("Not a valid base to create a unit.");
		return;
	}

	// �ڿ� üũ
	if (resource->spice < 5) {
		display_system_message("Not enough spice!");
		return;
	}

	// ���� ��ġ�� ���� ����
	POSITION base_pos = selected_cursor.current;
	POSITION spawn_pos = { -1, -1 }; // �⺻��: ��ġ �Ұ�
	int offset = 2; // �������� �� ĭ ������ ��ġ���� ��ġ ����

	if (base_pos.row >= MAP_HEIGHT / 2) {
		// ���ϴ� ����: ���� Ȯ��
		for (int row = base_pos.row - offset; row >= 1; row--) {
			if (map[1][row][base_pos.column] == -1 && map[0][row][base_pos.column] == ' ') {
				spawn_pos.row = row;
				spawn_pos.column = base_pos.column;
				break;
			}
		}
	}
	else {
		// ���� ����: �Ʒ��� Ȯ��
		for (int row = base_pos.row + offset; row < MAP_HEIGHT - 1; row++) {
			if (map[1][row][base_pos.column] == -1 && map[0][row][base_pos.column] == ' ') {
				spawn_pos.row = row;
				spawn_pos.column = base_pos.column;
				break;
			}
		}
	}

	// ��ȿ�� ��ġ ��ġ�� ���� ���
	if (spawn_pos.row == -1 || spawn_pos.column == -1) {
		display_system_message("No space to place the unit!");
		return;
	}

	// �Ϻ����� ����
	map[1][spawn_pos.row][spawn_pos.column] = 'H';
	resource->spice -= 5; // �ڿ� ����
	display_system_message("A new harvester ready!");

	//// ����� �޽��� ���
	//printf("DEBUG: Harvester placed at (%d, %d). Remaining spice: %d\n", spawn_pos.row, spawn_pos.column, resource->spice);
}












/* ================= ��ɾ� ó�� =================== */
void handle_spacebar(void) {
	selected_cursor.previous = selected_cursor.current;
	selected_cursor.current = cursor.current;

	/*printf("DEBUG: handle_spacebar called. Selected Cursor: (%d, %d)\n",
		selected_cursor.current.row, selected_cursor.current.column);*/

	char selected_tile = map[0][selected_cursor.current.row][selected_cursor.current.column];

	// ����(B)�� ���Ե� �������� Ȯ��
	if (selected_tile == 'B') {
		/*printf("DEBUG: Base selected\n");*/
	}
	else {
		/*display_system_message("Not a base!");*/
	}
}




void handle_esc(void) {
	selected_cursor.previous = selected_cursor.current;
	selected_cursor.current.row = -1;
	selected_cursor.current.column = -1;
	clear_status();
}