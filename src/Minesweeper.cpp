#include "Minesweeper.h"

using namespace minesweeper;

tile::tile(int x, int y, int type, states tflag = states::covered) : tile_x(x), tile_y(y), tile_type(type), tile_state(tflag) { }

// Flip flag state
void tile::flip_flag() {
	switch (tile_state) {
	case states::covered:
		tile_state = states::flagged;
		break;
	case states::flagged:
		tile_state = states::covered;
		break;
	}
}

// Initialize a game. Occurs when game state goes from new_game to in_progress
void MSGame::initialize_game(int initial_x, int initial_y) {
	remaining_uncleared = rows * columns - mines;
	int new_mines = 0;

	// The first tile clicked can't be a mine.
	// Ensure that as many tiles immediately adjacent to the first tile
	// are also not mines.
	int initial_tiles = std::min(rows * columns - mines - 1, 8);

	std::vector<int> arr(rows * columns, 0);

	for (auto vs = arr.begin(); vs < arr.end(); vs++) {
		*vs = static_cast<int>(vs - arr.begin());
	}

	fy_shuffle(arr, mines);

	for (int i = 0; i < (rows * columns) && new_mines < mines; i++) {
		int x = arr[i] % columns;
		int y = arr[i] / columns;

		if (initial_x == x && initial_y == y)
			continue;
		else if (std::abs(initial_x - x) <= 1 && std::abs(initial_y - y) <= 1 && initial_tiles > 0) {
			initial_tiles--;
			continue;
		}
		else {
			game_tiles[arr[i]].tile_type = -1;
			new_mines++;
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
	int new_x, new_y;

	for (int i = 0; i < 8; i++) {
		new_x = x + dx8[i];
		new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		if (get_tile_type(new_x, new_y) == -1)
			++entities;
	}

	return entities;
}

int MSGame::check_adjacent_flags(int x, int y) {
	int entities = 0;
	int new_x, new_y;

	for (int i = 0; i < 8; i++) {
		new_x = x + dx8[i];
		new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		if (get_tile_state(new_x, new_y) == states::flagged)
			++entities;
	}

	return entities;
}

void MSGame::reset() {
	game_tiles.clear();
	game_tiles.resize(rows * columns);
	remaining_mines = mines;
	current_state = g_states::new_game;
}

void MSGame::reset(int r, int c, int m) { // TODO: range checking on r, c and m

	rows = r;
	columns = c;
	mines = m;
	remaining_mines = mines;

	MSGame::reset();
}

// Apply right click operation
int MSGame::set_flag(int x, int y) {
	if (get_tile_state(x, y) == states::uncovered)
		return -1;

	if (get_tile_state(x, y) == states::covered) {
		game_tiles[y * columns + x].tile_state = states::flagged;
		dec_remaining();
	}
	else {
		game_tiles[y * columns + x].tile_state = states::covered;
		inc_remaining();
	}
	return 0;
}

// Apply double mouse click operation
std::vector<std::pair<int, int>> MSGame::d_click_clear(int x, int y) {
	if (get_tile_state(x, y) != states::uncovered)
		return {};

	int num_flagged = MSGame::check_adjacent_flags(x, y);
	if (get_tile_type(x, y) != num_flagged)
		return {};

	std::vector<std::pair<int, int>> uncovered_tiles;
	for (int i = 0; i < 8; i++) {
		int new_x = x + dx8[i], new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		// run left clicks on all adjacent tiles
		auto temp_vec = l_click_clear(new_x, new_y);
		if (!temp_vec.empty())
			uncovered_tiles.insert(uncovered_tiles.end(), temp_vec.begin(), temp_vec.end());
	}
	return uncovered_tiles;
}

// Apply left click operation
std::vector<std::pair<int, int>> MSGame::l_click_clear(int x, int y) {

	if (get_tile_state(x, y) != states::covered)
		return {};

	if (current_state == g_states::new_game) {
		initialize_game(x, y);
		current_state = g_states::in_progress;
	}

	if (get_tile_type(x, y) == -1) {
		game_tiles[y * columns + x].tile_type = -2;
		current_state = g_states::lost;
		return {};
	}

	if (get_tile_type(x, y) > 0) {
		game_tiles[y * columns + x].tile_state = states::uncovered;
		remaining_uncleared--;
		if (remaining_uncleared == 0)
			current_state = minesweeper::g_states::won;
		return { std::pair<int,int>{x, y} };
	}

	// reveal_adj is BFS helper function
	std::vector<std::pair<int, int>> uncovered_tiles;
	MSGame::reveal_adj(uncovered_tiles, x, y);
	remaining_uncleared -= uncovered_tiles.size();
	if (remaining_uncleared == 0)
		current_state = minesweeper::g_states::won;

	return uncovered_tiles;
}

void MSGame::reveal_adj(std::vector<std::pair<int, int>>& u_tiles, int x, int y) {

	if (x < 0 || y < 0 || x >= columns || y >= rows)
		return;
	if (get_tile_state(x, y) != states::covered || game_tiles[y * columns + x].tile_type == -1)
		return;

	u_tiles.push_back({ x,y });
	game_tiles[y * columns + x].tile_state = states::uncovered;

	if (get_tile_type(x, y) != 0)
		return;

	for (int i = 0; i < 8; i++) {
		reveal_adj(u_tiles, x + dx8[i], y + dy8[i]);
	}

	return;
}