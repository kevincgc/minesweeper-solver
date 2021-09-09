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

MSGame::MSGame(int r, int c, int m) : game_tiles(r* c), rows(r), columns(c), mines(m) {
	std::vector<int> arr(r * c, 0);

	for (auto vs = arr.begin(); vs < arr.end(); vs++) {
		*vs = *vs + static_cast<int>(vs - arr.begin());
	}

	fy_shuffle(arr, mines);

	for (int i = 0; i < mines; i++) {
		game_tiles[arr[i]].tile_type = -1;
	}

	for (int i = mines; i < (r * c); i++) {
		int x = arr[i] % columns;
		int y = arr[i] / columns;
		game_tiles[arr[i]].tile_type = check_adjacent_mines(x, y);
	}

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

	std::vector<int> arr(rows * columns, 0);

	for (auto vs = arr.begin(); vs < arr.end(); vs++) {
		*vs = *vs + static_cast<int>(vs - arr.begin());
	}

	fy_shuffle(arr, mines);

	for (int i = 0; i < mines; i++) {
		game_tiles[arr[i]].tile_type = -1;
	}

	for (int i = mines; i < (rows * columns); i++) {
		int x = arr[i] % columns;
		int y = arr[i] / columns;
		game_tiles[arr[i]].tile_type = check_adjacent_mines(x, y);
	}
}

void MSGame::reset(int r, int c, int m) { // TODO: range checking on r, c and m

	rows = r;
	columns = c;
	mines = m;

	MSGame::reset();
}
