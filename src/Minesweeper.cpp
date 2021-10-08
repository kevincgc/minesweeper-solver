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
	initialized = true;
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
}

// Initialize a game using a game code
void MSGame::initialize_game(std::string game_code) {
	initialized = true;

	int i = 0;
	while (true) {
		if (game_code[i] == ';')
			break;
		i++;
	}
	i++;
	while (true) {
		if (game_code[i] == ';')
			break;
		i++;
	}
	i++;

	std::string substr = game_code.substr(i);
	int total_tiles = rows * columns;
	int curr_tile_ind = 0;
	int total_mines = 0;

	for (int i = 0; i < substr.size(); i++) {
		if (curr_tile_ind >= total_tiles)
			break;

		int curr_num = 0;
		switch (substr[i]) {
		case '+':
			curr_num = 63;
			break;
		case '=':
			curr_num = 62;
			break;
		default:
			if (std::isdigit(substr[i]))
				curr_num = substr[i] - '0';
			else if (substr[i] >= 'a' && substr[i] <= 'z')
				curr_num = substr[i] - 'a' + 10;
			else
				curr_num = substr[i] - 'A' + 36;
			break;
		}

		int conv = 32;
		for (int j = 0; j < 6; j++) {
			if (curr_tile_ind >= total_tiles)
				break;

			if (curr_num >= conv) {
				game_tiles[curr_tile_ind].tile_type = -1;
				curr_num -= conv;
				total_mines++;
			}
			conv >>= 1;
			curr_tile_ind++;
		}
	}
	// If less code than needed, assume no mines

	for (int i = 0; i < (rows * columns); i++) {
		if (game_tiles[i].tile_type == -1)
			continue;

		int x = i % columns;
		int y = i / columns;
		game_tiles[i].tile_type = check_adjacent_mines(x, y);
	}

	mines = total_mines;
	remaining_mines = mines;
	remaining_uncleared = rows * columns - mines;
}

void MSGame::toggle_edit_mode(bool active) {
	if (active && current_state == g_states::edit)
		return;
	if (!active && current_state != g_states::edit)
		return;

	if (active) {
		if(current_state != g_states::editted)
			remaining_mines = mines;
		current_state = g_states::edit;
	}
	else {
		current_state = g_states::editted;
	}
}

// Check all adjacent tiles to count number of mines
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

// Check all adjacent tiles to count number of flagged tiles
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
	if (current_state != g_states::edit) {
		current_state = g_states::new_game;
		remaining_mines = mines;
	}
	else
		remaining_mines = 0;
	initialized = false;
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
	if (current_state == g_states::edit)	// do nothing if edit mode
		return {};

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

	if (current_state == g_states::edit) {	// if edit mode
		std::vector<std::pair<int, int>> changed_tiles;
		changed_tiles.push_back({ x, y });
		if (game_tiles[y * columns + x].tile_type < 0) { // currently mine, change to not mine
			remaining_mines--;
			game_tiles[y * columns + x].tile_type = check_adjacent_mines(x, y);
			for (int i = 0; i < 8; i++) {
				int new_x = x + dx8[i];
				int new_y = y + dy8[i];
				if (new_x >= 0 && new_x < columns
					&& new_y >= 0 && new_y < rows) {
					if (game_tiles[new_y * columns + new_x].tile_type < 0)
						continue;
					game_tiles[new_y * columns + new_x].tile_type = check_adjacent_mines(new_x, new_y);
					changed_tiles.push_back({ new_x, new_y });
				}
			}
		}
		else { // not a mine, change to mine
			remaining_mines++;
			game_tiles[y * columns + x].tile_type = -1;
			for (int i = 0; i < 8; i++) {
				int new_x = x + dx8[i];
				int new_y = y + dy8[i];
				if (new_x >= 0 && new_x < columns
					&& new_y >= 0 && new_y < rows) {
					if (game_tiles[new_y * columns + new_x].tile_type < 0)
						continue;
					game_tiles[new_y * columns + new_x].tile_type = check_adjacent_mines(new_x, new_y);
					changed_tiles.push_back({ new_x, new_y });
				}
			}
		}
		return changed_tiles;
	}

	if (get_tile_state(x, y) != states::covered)
		return {};

	if (current_state == g_states::new_game) {
		if(!initialized)
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

	// reveal_adj is DFS helper function
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

//Get game code
std::string MSGame::get_game_code() {
	if (current_state == g_states::new_game)
		return {};

	std::vector<int> raw_code;
	for (auto& a : game_tiles) {
		if (a.get_type() == -1)
			raw_code.push_back(1);
		else
			raw_code.push_back(0);
	}

	std::string result = std::to_string(get_rows()) + ';' + std::to_string(get_cols()) + ';';

	for (int i = 0; i < raw_code.size(); i = i + 6) {
		int agg_num = 0;
		for (int j = 0; j < 6; j++) {
			agg_num <<= 1;
			if (i + j < raw_code.size() && raw_code[i + j] == 1)
				agg_num += 1;
		}


		// 0-9: 0-9
		// a-z: 10-35
		// A-Z: 36-61
		// = : 62
		// + : 63

		if (agg_num < 10)
			result += std::to_string(agg_num);
		else if (agg_num < 36)
			result += ('a' + (agg_num - 10));
		else if (agg_num < 62)
			result += ('A' + (agg_num - 36));
		else if (agg_num == 62)
			result += '=';
		else
			result += '+';
	}

	return result;
}

bool minesweeper::check_code(std::string game_code, int& r, int& c, int& m) {
	if (game_code.size() == 0)
		return false;

	std::string substr = "";
	int i = 0;
	while (true) {
		if (i == game_code.size())
			return false;

		if (game_code[i] == ';')
			break;

		if (!std::isdigit(game_code[i]))
			return false;
		substr += game_code[i];
		i++;
	}
	if (substr.size() > 2 || substr.size()==0)
		return false;
	r = std::stoi(substr);
	if(r>99 || r<1)
		return false;
	i++;

	substr = "";
	while (true) {
		if (i == game_code.size())
			return false;

		if (game_code[i] == ';')
			break;

		if (!std::isdigit(game_code[i]))
			return false;
		substr += game_code[i];
		i++;
	}
	if (substr.size() > 2 || substr.size() == 0)
		return false;
	c = std::stoi(substr);
	if (c > 99 || c < 8)
		return false;
	i++;

	if (i == game_code.size())
		return false;

	substr = "";
	while (i < game_code.size()) {
		if (!std::isalnum(game_code[i]) && game_code[i] != '+' && game_code[i] != '=')
			return false;

		substr += game_code[i];
		i++;
	}

	int curr_tile_ind = 0;
	int total_tiles = r * c;
	for (int i = 0; i < substr.size(); i++) {
		if (curr_tile_ind >= total_tiles)
			break;

		int curr_num = 0;
		switch (substr[i]) {
		case '+':
			curr_num = 63;
			break;
		case '=':
			curr_num = 62;
			break;
		default:
			if (std::isdigit(substr[i]))
				curr_num = substr[i] - '0';
			else if (substr[i] >= 'a' && substr[i] <= 'z')
				curr_num = substr[i] - 'a' + 10;
			else
				curr_num = substr[i] - 'A' + 36;
			break;
		}

		int conv = 32;
		for (int j = 0; j < 6; j++) {
			if (curr_tile_ind >= total_tiles)
				break;

			if (curr_num >= conv) {
				m++;
				curr_num -= conv;
			}
			conv >>= 1;
			curr_tile_ind++;
		}
	}

	return true;
}