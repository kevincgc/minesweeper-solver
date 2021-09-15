#include <iostream>
#include <random>
#include <iterator>
#include <vector>
#include <chrono>
#include "Minesweeper.h"


using namespace minesweeper;

tile::tile(int x, int y, int type, states tflag = states::covered) : tile_x(x), tile_y(y), tile_type(type), tile_status(tflag) { }

void tile::flip_flag() {
	switch (tile_status) {
	case states::covered:
		tile_status = states::flagged;
		break;
	case states::flagged:
		tile_status = states::covered;
		break;
	}
}

void MSGame::initialize_game(int initial_x, int initial_y) {
	remaining_mines = 0;
	remaining_uncleared = rows * columns - mines;
	int initial_tiles = std::min(rows * columns - mines - 1, 8);

	std::vector<int> arr(rows * columns, 0);

	for (auto vs = arr.begin(); vs < arr.end(); vs++) {
		*vs = static_cast<int>(vs - arr.begin());
	}

	fy_shuffle(arr, mines);

	for (int i = 0; i < (rows * columns) && initial_tiles > 0 && remaining_mines < mines; i++) {
		int x = arr[i] % columns;
		int y = arr[i] / columns;

		if (initial_x == x && initial_y == y)
			continue;
		else if (std::abs(initial_x - x) <= 1 && std::abs(initial_y - y) <= 1) {
			initial_tiles--;
			continue;
		}
		else {
			game_tiles[arr[i]].tile_type = -1;
			remaining_mines++;
		}
	}

	for (int i = 0; i < (rows * columns); i++) {
		if (game_tiles[arr[i]].tile_type == -1)
			continue;

		int x = arr[i] % columns;
		int y = arr[i] / columns;
		game_tiles[arr[i]].tile_type = check_adjacent_mines(x, y);
	}

	remaining_uncleared = rows * columns - mines;
}

int MSGame::check_adjacent_mines(int x, int y) {
	int entities = 0;
	for (int i = -1; i <= 1; i++) {
		int tempcoord = y + i;
		if (tempcoord < 0 || tempcoord >= rows) continue;

		for (int j = -1; j <= 1; j++) {
			int tempcoord = x + j;
			if (tempcoord < 0 || tempcoord >= columns) continue;

			if (game_tiles[x + j + ((y + i) * columns)].tile_type == -1)
				++entities;
		}
	}

	return entities;
}

int MSGame::check_adjacent_flags(int x, int y) {
	int entities = 0;
	for (int i = -1; i <= 1; i++) {
		int tempcoord = y + i;
		if (tempcoord < 0 || tempcoord >= rows) continue;

		for (int j = -1; j <= 1; j++) {
			int tempcoord = x + j;
			if (tempcoord < 0 || tempcoord >= columns) continue;

			if (game_tiles[x + j + ((y + i) * columns)].tile_status == states::flagged)
				++entities;
		}
	}

	return entities;
}

void MSGame::reset() {
	game_tiles.clear();
	game_tiles.resize(rows * columns);
	current_state = g_states::new_game;
}

void MSGame::reset(int r, int c, int m) { // TODO: range checking on r, c and m

	rows = r;
	columns = c;
	mines = m;

	MSGame::reset();
}

int MSGame::set_flag(int x, int y) {
	if (game_tiles[y * columns + x].tile_status == states::uncovered)
		return -1;

	if (game_tiles[y * columns + x].tile_status == states::covered)
		game_tiles[y * columns + x].tile_status = states::flagged;
	else
		game_tiles[y * columns + x].tile_status = states::covered;
	return 0;
}

std::vector<std::pair<int, int>> MSGame::d_click_clear(int x, int y) {
	if (game_tiles[y * columns + x].tile_status != states::uncovered)
		return {};

	int num_flagged = MSGame::check_adjacent_flags(x, y);
	if (game_tiles[y * columns + x].tile_type != num_flagged)
		return {};

	std::vector<std::pair<int, int>> uncovered_tiles;
	for (int i = 0; i < 8; i++) {
		if (x + dx8[i] < 0 || y + dy8[i] < 0 || x + dx8[i] >= columns || y + dy8[i] >= rows)
			continue;

		auto temp_vec = l_click_clear(x + dx8[i], y + dy8[i]);
		if (!temp_vec.empty())
			uncovered_tiles.insert(uncovered_tiles.end(), temp_vec.begin(), temp_vec.end());
	}
	remaining_uncleared -= uncovered_tiles.size();
	return uncovered_tiles;

}
//TODO, move the fy shuffle to after the first click, to ensure possible first click
//TODO, implement win condition checking
std::vector<std::pair<int, int>> MSGame::l_click_clear(int x, int y) {

	if (current_state == g_states::new_game) {
		initialize_game(x, y);
		current_state = g_states::in_progress;
	}

	if (game_tiles[y * columns + x].tile_status != states::covered)
		return {};

	if (game_tiles[y * columns + x].tile_type == -1) {
		game_tiles[y * columns + x].tile_type = -2;
		current_state = g_states::lost;
		return {};
	}

	if (game_tiles[y * columns + x].tile_type > 0) {
		game_tiles[y * columns + x].tile_status = states::uncovered;
		remaining_uncleared--;
		return { std::pair<int,int>{x, y} };
	}

	std::vector<std::pair<int, int>> uncovered_tiles;
	MSGame::reveal_adj(uncovered_tiles, x, y);
	remaining_uncleared -= uncovered_tiles.size();
	return uncovered_tiles;
}

void MSGame::reveal_adj(std::vector<std::pair<int, int>>& u_tiles, int x, int y) {

	if (x < 0 || y < 0 || x >= columns || y >= rows)
		return;
	if (game_tiles[y * columns + x].tile_status != states::covered || game_tiles[y * columns + x].tile_type == -1)
		return;

	u_tiles.push_back({ x,y });
	game_tiles[y * columns + x].tile_status = states::uncovered;

	if (game_tiles[y * columns + x].tile_type != 0)
		return;

	for (int i = 0; i < 8; i++) {
		reveal_adj(u_tiles, x + dx8[i], y + dy8[i]);
	}

	return;
}