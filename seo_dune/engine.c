#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"

// 외부에서 선언된 selected_cursor 변수를 참조
extern CURSOR selected_cursor;
extern const POSITION map_pos; // 외부에서 참조할 수 있도록 선언

/* ================= 함수 선언 =================== */
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

SANDWORM sandworms[10];      // 최대 10개의 샌드웜 관리
int sandworm_count = 0;      // 현재 샌드웜 수

// 샌드웜 초기화
void init_sandworm(SANDWORM* worm, POSITION start_pos, int speed, int deposit_period) {
	worm->pos = start_pos;
	worm->next_move_time = sys_clock + speed;
	worm->speed = speed;

	// 배설 주기를 랜덤화
	worm->deposit_period = deposit_period; // 수정된 부분
	worm->next_deposit_time = sys_clock + worm->deposit_period;

	worm->target = (POSITION){ -1, -1 }; // 초기화: 목표 없음
	/*printf("DEBUG: Initialized Sandworm at (%d, %d) with deposit period = %dms\n",
		worm->pos.row, worm->pos.column, worm->deposit_period);*/
}




// 샌드웜 초기화 (맵의 W 위치 기반)
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


void update_sandworms(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], RESOURCE* resource) {
	for (int i = 0; i < sandworm_count; i++) {
		SANDWORM* worm = &sandworms[i];

		// 이동 처리: 목표가 유효한 경우 이동
		if (worm->target.row == -1 || worm->target.column == -1 ||
			map[1][worm->target.row][worm->target.column] != 'H') {
			worm->target = find_closest_unit(map, worm->pos);

			if (worm->target.row == -1 || worm->target.column == -1) {
				continue;
			}
		}

		// 이동
		move_sandworm(worm, map);

		// 샌드웜이 하베스터 위치에 도달했는지 확인
		if (map[1][worm->pos.row][worm->pos.column] == 'H') {
			// 하베스터 제거
			map[1][worm->pos.row][worm->pos.column] = -1;

			// 메시지 출력
			char message[100]; // 메시지 버퍼 크기 설정
			snprintf(message, sizeof(message),
				"하베스터가 (%d, %d)에서 샌드웜에게 잡혔습니다!",
				worm->pos.row, worm->pos.column);
			display_system_message(message);
			// 자원 감소 등 추가 로직 처리 가능
			resource->population--; // 인구 감소
		}

		// 배설 처리 (이동 후 배설)
		if (sys_clock >= worm->next_deposit_time) {
			deposit_spice(worm, map);
		}
	}
}





// 전역 변수
bool is_building = false; // 건설 모드 여부
char selected_building = '\0'; // 선택된 건물 ('P', 'B', 등)
int build_timer = 0; // 건설 타이머
int cursor_size = 1; // 커서 크기 (1x1 또는 2x2)

// 건설 모드 진입
void enter_build_mode() {
	is_building = true; // 건설 모드 활성화
	selected_building = '\0'; // 아직 건물 선택 안 됨
	display_system_message("건설 모드: P (장판), D (숙소), G (창고)"); // 명령창에 메시지 표시
}



// 건물 선택
void select_building(char building_type) {
	selected_building = building_type;
	cursor_size = 2; // 커서를 2x2로 변경
	display_system_message("건물을 선택했습니다. 스페이스바로 건설 시작.");
}

// 건설 시작
void start_building() {
	if (selected_building == '\0') {
		display_system_message("건설할 건물을 선택하세요!");
		return;
	}
	///asasasasasas
	// 건설 위치 확인 및 타이머 시작
	POSITION pos = cursor.current;
	for (int i = 0; i < cursor_size; i++) {
		for (int j = 0; j < cursor_size; j++) {
			POSITION check_pos = { pos.row + i, pos.column + j };
			if (check_pos.row < 1 || check_pos.row >= MAP_HEIGHT - 1 ||
				check_pos.column < 1 || check_pos.column >= MAP_WIDTH - 1) {
				display_system_message("건설할 위치가 맵을 벗어났습니다!");
				return;
			}
			if (map[0][check_pos.row][check_pos.column] != 'P') {
				display_system_message("건설할 위치에 장판이 없습니다!");
				return;
			}
		}
	}

	build_timer = 300; // 3초 동안 건설
	char timer_message[40];
	int remaining_time = build_timer * TICK / 1000;
	snprintf(timer_message, sizeof(timer_message), "건설 중... 남은 시간: %d초", remaining_time);
	display_system_message(timer_message); // 초기 메시지 출력

	display(resource, map, cursor); // 화면 갱신
}



// 건설 취소
void cancel_building() {
	/*printf("DEBUG: cancel_building() 호출됨. 초기 상태 - is_building: %d, build_timer: %d\n", is_building, build_timer);*/
	is_building = false;           // 건설 모드 종료
	selected_building = '\0';      // 선택된 건물 초기화
	build_timer = 0;               // 건설 타이머 초기화
	cursor_size = 1;               // 커서를 1x1로 복구
	/*display_system_message("DEBUG: 건설 취소 실행됨.");*/
	/*printf("DEBUG: cancel_building() 실행 완료. 최종 상태 - is_building: %d, build_timer: %d\n", is_building, build_timer);*/
}






// 건설 완료
void complete_building() {
	POSITION pos = cursor.current;

	// 선택된 위치에 건물 설치
	for (int i = 0; i < cursor_size; i++) {
		for (int j = 0; j < cursor_size; j++) {
			POSITION build_pos = { pos.row + i, pos.column + j };
			map[0][build_pos.row][build_pos.column] = selected_building;
		}
	}

	// 상태 초기화
	is_building = false;
	selected_building = '\0';
	cursor_size = 1; // 커서를 원래 크기로 복구
	display_system_message("건물이 성공적으로 설치되었습니다!");
	display(resource, map, cursor); // 화면 갱신
}






/* ================= 추가 변수 =================== */
KEY last_key = k_none;        // 마지막 입력된 키를 저장
clock_t last_key_time = 0;    // 마지막 키 입력 시간을 저장
int double_click_delay = 200; // 더블클릭 인식 시간(밀리초 단위)
int multi_move_distance = 5;  // 더블클릭 시 이동할 칸 수

/* ================= main() =================== */
int main(void) {
	srand((unsigned int)time(NULL));

	init();
	init_sandworms(map);  // 샌드웜 초기화
	intro();
	display(resource, map, cursor);

	char last_timer_message[40] = "";  // 이전에 출력한 메시지를 저장

	while (1) {
		KEY key = get_key();

		// x 키를 먼저 처리
		if (key == 'X' || key == 'x') {
			/*printf("DEBUG: x 키 입력됨. is_building = %d, build_timer = %d\n", is_building, build_timer);*/
			if (is_building && build_timer > 0) {
				// 건설 진행 중 취소
				/*printf("DEBUG: 건설 진행 중 취소 로직 실행\n");*/
				cancel_building(); // 건설 취소
				display_system_message("건설 진행 중 취소되었습니다.");
				display(resource, map, cursor); // 화면 갱신
				continue; // 다음 루프로 이동
			}
			else if (is_building) {
				// 건설 모드만 취소
				/*printf("DEBUG: 건설 모드 취소 로직 실행\n");*/
				is_building = false;
				selected_building = '\0';
				cursor_size = 1; // 커서를 1x1로 되돌림
				display_system_message("건설 모드가 취소되었습니다.");
				display(resource, map, cursor); // 화면 갱신
				continue; // 다음 루프로 이동
			}
			else {
				/*printf("DEBUG: 건설 상태가 아님. x 키 입력 무시됨\n");*/
				display_system_message("건설 상태가 아니어서 취소할 수 없습니다.");
				continue; // 다음 루프로 이동
			}
		}

		// 건설 상태 관리
		if (is_building) {
			if (build_timer > 0) {
				build_timer--; // 건설 타이머 감소
				/*printf("DEBUG: build_timer = %d\n", build_timer);*/

				// 남은 시간 계산 및 표시
				char timer_message[40];
				int remaining_time = build_timer * TICK / 1000; // 남은 시간 (초 단위)
				snprintf(timer_message, sizeof(timer_message), "건설 중... 남은 시간: %d초", remaining_time);

				// 이전 메시지와 다를 경우에만 출력
				if (strcmp(timer_message, last_timer_message) != 0) {
					display_system_message(timer_message);
					strncpy_s(last_timer_message, sizeof(last_timer_message), timer_message, _TRUNCATE);
				}

				// 깜박임 효과 (건설 중인 영역)
				for (int i = 0; i < cursor_size; i++) {
					for (int j = 0; j < cursor_size; j++) {
						POSITION build_pos = { cursor.current.row + i, cursor.current.column + j };
						if (build_pos.row >= 1 && build_pos.row < MAP_HEIGHT - 1 &&
							build_pos.column >= 1 && build_pos.column < MAP_WIDTH - 1) {
							char ch = frontbuf[build_pos.row][build_pos.column];
							int color = (sys_clock % 500 < 250) ? COLOR_CURSOR : COLOR_DEFAULT; // 500ms 주기
							printc(padd(map_pos, build_pos), ch, color);
						}
					}
				}

				set_color(COLOR_DEFAULT); // 색상을 항상 복구

				// 건설 완료
				if (build_timer == 0) {
					complete_building();
					is_building = false; // 건설 모드 종료
				}

				// 시간 경신 및 대기
				Sleep(TICK);
				sys_clock += TICK;
				continue; // 건설 상태 처리 후 다음 루프로 이동
			}
		}

		// 키 입력 처리
		if (is_arrow_key(key)) {
			clock_t current_time = clock();
			int time_diff = (int)((current_time - last_key_time) * 1000 / CLOCKS_PER_SEC);

			if (key == last_key && time_diff <= double_click_delay) {
				// 더블클릭: 멀리 이동
				for (int i = 0; i < multi_move_distance; i++) {
					cursor_move(ktod(key));
				}
			}
			else {
				// 단일 이동
				cursor_move(ktod(key));
			}

			last_key = key;
			last_key_time = current_time;
		}
		else {
			// 기타 키 입력 처리
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
						display_system_message("건설할 건물을 선택하세요!");
					}
					else {
						start_building(); // 건설 시작
					}
				}
				else {
					handle_spacebar(); // 일반 상태의 스페이스바 동작
				}
				break;
			case 'B':
			case 'b': // 건설 모드 진입
				if (!is_building) {
					enter_build_mode();
				}
				break;
			case 'H':
			case 'h': // 소문자 h도 처리
				create_unit_at_base(&resource, map, selected_cursor);
				break;
			case 'P':
			case 'p': // 건설할 장판 선택
			case 'D':
			case 'd': // 건설할 숙소 선택
			case 'G':
			case 'g': // 건설할 창고 선택
				if (is_building) {
					select_building(key);
				}
				break;
			default:
				break;
			}
		}

		// 샘플 오브젝트 동작
		sample_obj_move();

		// 샌드웜 업데이트: 목표가 없을 경우 추가 처리 없음
		update_sandworms(map, &resource);

		// 단일 화면 갱신
		display(resource, map, cursor);

		// 추가: 커서를 맨 아래로 이동 후 화면을 깔끔히 처리
		gotoxy((POSITION) { MAP_HEIGHT + 5, 0 });
		printf("\n"); // 명확히 줄바꿈 추가

		// 시스템 시간 증가
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
	// 초기 상태 설정 [UPDATED WITH BOUNDARY IN MIND]
	// 좌하단 본진(B) 배치 (2x2 형태로 위치 조정)
	map[0][MAP_HEIGHT - 3][1] = 'B';
	map[0][MAP_HEIGHT - 3][2] = 'B';
	map[0][MAP_HEIGHT - 2][1] = 'B';
	map[0][MAP_HEIGHT - 2][2] = 'B';
	// 좌하단 장판(P) 배치
	map[0][MAP_HEIGHT - 3][3] = 'P';
	map[0][MAP_HEIGHT - 2][3] = 'P';
	map[0][MAP_HEIGHT - 3][4] = 'P';
	map[0][MAP_HEIGHT - 2][4] = 'P';
	// 우상단 본진(B) 배치
	map[0][1][MAP_WIDTH - 2] = 'B';
	map[0][2][MAP_WIDTH - 2] = 'B';
	map[0][1][MAP_WIDTH - 3] = 'B';
	map[0][2][MAP_WIDTH - 3] = 'B';
	// 우상단 장판 배치
	map[0][1][MAP_WIDTH - 5] = 'P';
	map[0][2][MAP_WIDTH - 5] = 'P';
	map[0][1][MAP_WIDTH - 4] = 'P';
	map[0][2][MAP_WIDTH - 4] = 'P';
	// 좌하단 하베스터(H) 배치   
	map[1][MAP_HEIGHT - 4][1] = 'H';

	// 우상단 하베스터(H) 배치
	map[1][3][MAP_WIDTH - 2] = 'H';

	// 좌하단 스파이스(5) 배치
	map[0][MAP_HEIGHT - 6][1] = 'S';

	// 우상단 스파이스(5) 배치
	map[0][5][MAP_WIDTH - 2] = 'S';

	// 바위(R) 배치 (맵 내 곳곳에 배치, 테두리 고려)
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

	// 샌드웜 생성
	map[1][2][10] = 'W';
	map[1][13][45] = 'W';

	// layer 0(map[0])에 지형 생성
	// 외곽 벽 생성
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}
	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
	}

	// 빈 공간 초기화
	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			if (map[0][i][j] == 0) {
				map[0][i][j] = ' ';
			}
		}
	}

	// layer 1(map[1])은 비워 두기(-1로 채움)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			if (map[1][i][j] == 0) {
				map[1][i][j] = -1;
			}
		}
	}


	// 초기 자원 설정
	resource.spice = 15;
	resource.spice_max = 100;
	resource.population = 2;
	resource.population_max = 50;

	// 초기 selected_cursor 위치를 커서 위치로 설정
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
//야 박주은
//test

void sample_obj_move(void) {
	if (sys_clock <= obj.next_move_time) return;

	map[1][obj.pos.row][obj.pos.column] = -1; // 기존 위치 초기화
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}

// 맨해튼 거리 계산
int manhattan_distance(POSITION p1, POSITION p2) {
	return abs(p1.row - p2.row) + abs(p1.column - p2.column);
}


// 가장 가까운 유닛 찾기
POSITION find_closest_unit(char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], POSITION sandworm_pos) {
	POSITION closest = { -1, -1 }; // 기본값: 유닛 없음
	int min_distance = INT_MAX;

	for (int row = 0; row < MAP_HEIGHT; row++) {
		for (int col = 0; col < MAP_WIDTH; col++) {
			if (map[1][row][col] == 'H') { // 유닛(H) 발견
				// 맨해튼 거리 계산
				int distance = manhattan_distance(sandworm_pos, (POSITION) { row, col });

				// 유효한 이동 경로인지 확인
				if (map[0][row][col] != 'S' && map[0][row][col] != 'R' && map[0][row][col] != '#' &&
					distance < min_distance) {
					closest.row = row;
					closest.column = col;
					min_distance = distance;
				}
			}
		}
	}

	//// 디버깅 메시지
	//if (closest.row == -1 && closest.column == -1) {
	//	printf("DEBUG: No harvester found for sandworm at (%d, %d)\n", sandworm_pos.row, sandworm_pos.column);
	//}
	//else {
	//	printf("DEBUG: Closest harvester to sandworm at (%d, %d) is at (%d, %d), distance = %d\n",
	//		sandworm_pos.row, sandworm_pos.column, closest.row, closest.column, min_distance);
	//}

	return closest;
}






// 샌드웜 이동
void move_sandworm(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	if (sys_clock < worm->next_move_time) return; // 이동 제한 시간 확인

	POSITION diff = psub(worm->target, worm->pos);
	DIRECTION dir;

	// 방향 결정 (세로 우선)
	if (abs(diff.row) > abs(diff.column)) {
		dir = (diff.row > 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column > 0) ? d_right : d_left;
	}

	POSITION next_pos = pmove(worm->pos, dir);

	// 이동 가능 여부 확인
	if (next_pos.row > 0 && next_pos.row < MAP_HEIGHT - 1 &&
		next_pos.column > 0 && next_pos.column < MAP_WIDTH - 1 &&
		map[0][next_pos.row][next_pos.column] == ' ') {

		// 이전 위치 초기화
		map[1][worm->pos.row][worm->pos.column] = -1;

		// 새 위치로 이동
		worm->pos = next_pos;
		map[1][worm->pos.row][worm->pos.column] = 'W';

		/*printf("DEBUG: Sandworm moved to (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}
	else {
		/*printf("DEBUG: Movement blocked for Sandworm at (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}

	// 다음 이동 시간 갱신
	worm->next_move_time = sys_clock + worm->speed;
}



















// 스파이스 배설 (배설 주기 랜덤화)
void deposit_spice(SANDWORM* worm, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH]) {
	if (sys_clock < worm->next_deposit_time) return;

	// 현재 위치가 빈 공간인지 확인
	if (map[0][worm->pos.row][worm->pos.column] == ' ') {
		map[0][worm->pos.row][worm->pos.column] = 'S'; // 스파이스 배치
		/*printf("DEBUG: Sandworm deposited spice at (%d, %d)\n", worm->pos.row, worm->pos.column);*/
	}

	worm->next_deposit_time = sys_clock + worm->deposit_period; // 다음 배설 시간 설정
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











/* ================= 유닛 생성 함수 =================== */
void create_unit_at_base(RESOURCE* resource, char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH], CURSOR selected_cursor) {
	// 본진에서 하베스터를 배치할 타일을 찾기
	char selected_tile = map[0][selected_cursor.current.row][selected_cursor.current.column];
	if (selected_tile != 'B') {
		display_system_message("Not a valid base to create a unit.");
		return;
	}

	// 자원 체크
	if (resource->spice < 5) {
		display_system_message("Not enough spice!");
		return;
	}

	// 본진 위치와 방향 설정
	POSITION base_pos = selected_cursor.current;
	POSITION spawn_pos = { -1, -1 }; // 기본값: 배치 불가
	int offset = 2; // 본진에서 두 칸 떨어진 위치부터 배치 시작

	if (base_pos.row >= MAP_HEIGHT / 2) {
		// 좌하단 본진: 위로 확장
		for (int row = base_pos.row - offset; row >= 1; row--) {
			if (map[1][row][base_pos.column] == -1 && map[0][row][base_pos.column] == ' ') {
				spawn_pos.row = row;
				spawn_pos.column = base_pos.column;
				break;
			}
		}
	}
	else {
		// 우상단 본진: 아래로 확장
		for (int row = base_pos.row + offset; row < MAP_HEIGHT - 1; row++) {
			if (map[1][row][base_pos.column] == -1 && map[0][row][base_pos.column] == ' ') {
				spawn_pos.row = row;
				spawn_pos.column = base_pos.column;
				break;
			}
		}
	}

	// 유효한 배치 위치가 없는 경우
	if (spawn_pos.row == -1 || spawn_pos.column == -1) {
		display_system_message("No space to place the unit!");
		return;
	}

	// 하베스터 생성
	map[1][spawn_pos.row][spawn_pos.column] = 'H';
	resource->spice -= 5; // 자원 차감
	display_system_message("A new harvester ready!");

	//// 디버그 메시지 출력
	//printf("DEBUG: Harvester placed at (%d, %d). Remaining spice: %d\n", spawn_pos.row, spawn_pos.column, resource->spice);
}












/* ================= 명령어 처리 =================== */
void handle_spacebar(void) {
	selected_cursor.previous = selected_cursor.current;
	selected_cursor.current = cursor.current;

	/*printf("DEBUG: handle_spacebar called. Selected Cursor: (%d, %d)\n",
		selected_cursor.current.row, selected_cursor.current.column);*/

	char selected_tile = map[0][selected_cursor.current.row][selected_cursor.current.column];

	// 본진(B)이 포함된 영역인지 확인
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