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

int MSGame::check_adjacent_covered(int x, int y) {
	int entities = 0;
	int new_x, new_y;

	for (int i = 0; i < 8; i++) {
		new_x = x + dx8[i];
		new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		if (get_tile_state(new_x, new_y) == states::covered)
			++entities;
	}

	return entities;
}

int MSGame::check_adjacent_uncovered(int x, int y) {
	int entities = 0;
	int new_x, new_y;

	for (int i = 0; i < 8; i++) {
		new_x = x + dx8[i];
		new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		if (get_tile_state(new_x, new_y) == states::uncovered)
			++entities;
	}

	return entities;
}

int MSGame::check_adjacent_tiles(int x, int y) {
	int entities = 0;
	int new_x, new_y;

	for (int i = 0; i < 8; i++) {
		new_x = x + dx8[i];
		new_y = y + dy8[i];

		if (new_x < 0 || new_y < 0 || new_x >= columns || new_y >= rows)
			continue;

		if (get_tile_state(new_x, new_y) == states::uncovered || get_tile_state(new_x, new_y) == states::covered || get_tile_state(new_x, new_y) == states::flagged)
			++entities;
	}

	return entities;
}

int MSGame::check_remaining_mines(int x, int y) {
	int entities = 0;
	int new_x, new_y;
	int flagged = check_adjacent_flags(x, y);
	int mines = get_tile_type(x, y);
	if (mines < 0) {
		std::cout << "you dun goofed" << std::endl;
		return -1;
	}

	return mines - flagged;
}

void MSGame::reset() {
	game_tiles.clear();
	game_tiles.resize(rows * columns);
	p_mat.resize(rows * columns);
	if (current_state != g_states::edit) {
		current_state = g_states::new_game;
		remaining_mines = mines;
	}
	else
		remaining_mines = 0;
	initialized = false;
	//if (p_mat != nullptr)
	//	delete(p_mat);
	//p_mat = (float**)malloc(columns * sizeof(float*));
	//for (int i = 0; i < columns; i++)
	//	p_mat[i] = (float*)malloc(rows * sizeof(float));
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
	if (get_tile_state(x, y) == states::flagged)
		return 0;

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

void minesweeper::MSGame::solver_step()
{
	//int covered = count_covered();
	//calc_p_mat();
	//for (int x = 0; x < columns; x++) {
	//	for (int y = 0; y < rows; y++) {
	//		if (fequals(p_mat[get_index_from_xy(x, y)], 1) && get_tile_state(x, y) != states::flagged) {
	//			set_flag(x, y);
	//			return;
	//		}
	//	}
	//}
	//std::vector<std::pair<int, int>> min_p_tuples = get_min_p_tuples();
	//std::srand(std::time(nullptr));
	//int i = std::rand() % min_p_tuples.size();
	//l_click_clear(std::get<0>(min_p_tuples[i]), std::get<1>(min_p_tuples[i]));
	return;
}

int minesweeper::MSGame::get_index_from_xy(int x, int y) {
	return y * columns + x;
}

bool fequals(float a, float b)
{
	return fabs(a - b) < 1e-5;
}

std::pair<int, int> minesweeper::MSGame::find_d_clear() {
	for (int x_center = 0; x_center < columns; x_center++) {
		for (int y_center = 0; y_center < rows; y_center++) {
			if (get_tile_state(x_center, y_center) == states::covered || get_tile_state(x_center, y_center) == states::flagged) continue;
			int adjacent_mines = get_tile_type(x_center, y_center);
			if (adjacent_mines == 0) continue;
			int flagged = check_adjacent_flags(x_center, y_center);
			int uncovered = check_adjacent_uncovered(x_center, y_center);
			int tiles = check_adjacent_tiles(x_center, y_center);
			if (adjacent_mines == flagged && tiles > uncovered + flagged)
				return { x_center,y_center };
		}
	}
	return { -1,-1 };
}

std::pair<int,int> minesweeper::MSGame::find_mine() {
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			if (fequals(p_mat[get_index_from_xy(x, y)], 1) && get_tile_state(x, y) != states::flagged && get_tile_state(x, y) != states::uncovered) {
				return { x,y };
			}
		}
	}
	return { -1,-1 };
}

void minesweeper::MSGame::clear_p_mat()
{
	for (int i = 0; i < p_mat.size(); i++) {
		p_mat[i] = -1;
	}
}

std::vector<std::pair<int, int>> minesweeper::MSGame::get_min_p_tuples()
{
	std::vector<std::pair<int, int>> min_p_tuples = std::vector<std::pair<int, int>>();
	float min_p = 0.99;
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			if (get_tile_state(x, y) == states::uncovered || get_tile_state(x, y) == states::flagged)
				continue;
			if (fequals(p_mat[get_index_from_xy(x, y)], 0)) {
				min_p_tuples.clear();
				min_p_tuples.push_back({ x,y });
				return min_p_tuples;
			}
			if (p_mat[get_index_from_xy(x, y)] < min_p) {
				min_p = p_mat[get_index_from_xy(x, y)];
				min_p_tuples.clear();
				min_p_tuples.push_back({ x,y });
			} else if (p_mat[get_index_from_xy(x, y)] == min_p)
				min_p_tuples.push_back({ x,y });
		}
	}
	return min_p_tuples;
}

int minesweeper::MSGame::count_covered() {
	int n = 0;
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			if (get_tile_state(x, y) == states::covered)
				n++;
		}
	}
	return n;
}

void minesweeper::MSGame::set_adjacent_p(int x_center, int y_center)
{
	int adjacent_mines = get_tile_type(x_center, y_center);
	if (adjacent_mines == 0) return;
	int flagged = check_adjacent_flags(x_center, y_center);
	int uncovered = check_adjacent_uncovered(x_center, y_center);
	int tiles = check_adjacent_tiles(x_center, y_center);
	if (tiles == uncovered + flagged) return;
	float p = (float)(adjacent_mines - flagged) / (tiles - uncovered - flagged);
	for (int x = x_center - 1; x <= x_center + 1; x++) {
		for (int y = y_center - 1; y <= y_center + 1; y++) {
			if (x >= 0 && x < columns && y >= 0 && y < rows && !(x == x_center && y == y_center)) {
				if (get_tile_state(x, y) == states::covered)
					if (p > p_mat[get_index_from_xy(x, y)] && !fequals(p_mat[get_index_from_xy(x, y)], 0))
						p_mat[get_index_from_xy(x, y)] = p;
			}
		}
	}
}

bool minesweeper::MSGame::is_subset(std::vector<std::pair<int, int>> other, std::vector<std::pair<int, int>> superset) {
	std::set<std::pair<int, int>> s;
	if (other.size() == 0 || superset.size() == 0)
		return false;
	if ((int)superset.size() - (int)other.size() != 1)
		return false;
	for (auto e: superset)
		s.insert(e);
	for (auto e : other)
		s.insert(e);
	if (s.size() == superset.size())
		return true;
	return false;
	
}

std::tuple<int, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>> minesweeper::MSGame::find_overlap() {
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			if (get_tile_state(x, y) == states::uncovered && get_tile_type(x, y) > 0) {
				std::tuple<int, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>> overlap = find_overlap_for_tile(x, y);
				if (get<0>(overlap) != -1) {
					return overlap;
				}
			}
		}
	}
	return { -1, {-1,-1}, {-1,-1}, {-1,-1} };
}

std::tuple<int, std::pair<int, int>, std::pair<int, int>, std::pair<int, int>> minesweeper::MSGame::find_overlap_for_tile(int x_center, int y_center) {
	std::vector<std::pair<int, int>> adjacent_covered;
	for (int x = x_center - 1; x <= x_center + 1; x++) {
		for (int y = y_center - 1; y <= y_center + 1; y++) {
			if (x >= 0 && x < columns && y >= 0 && y < rows && !(x == x_center && y == y_center)) {
				if (get_tile_state(x, y) == states::covered)
					adjacent_covered.push_back({ x,y });
			}
		}
	}
	for (int x = x_center - 1; x <= x_center + 1; x++) {
		for (int y = y_center - 1; y <= y_center + 1; y++) {
			if (x >= 0 && x < columns && y >= 0 && y < rows && !(x == x_center && y == y_center)) {
				if (get_tile_type(x, y) <= 0 || get_tile_state(x, y) == states::covered || get_tile_state(x, y) == states::flagged)
					continue;
				std::vector<std::pair<int, int>> adjacent_covered_other;
				for (int x_other = x - 1; x_other <= x + 1; x_other++) {
					for (int y_other = y - 1; y_other <= y + 1; y_other++) {
						if (x_other >= 0 && x_other < columns && y_other >= 0 && y_other < rows && !(x_other == x_center && y_other == y_center) && !(x_other == x && y_other == y)) {
							if (get_tile_state(x_other, y_other) == states::covered)
								adjacent_covered_other.push_back({ x_other,y_other });
						}
					}
				}
				if (is_subset(adjacent_covered, adjacent_covered_other)) { // adjacent_covered_other is superset
					int adjacent_mines = check_remaining_mines(x_center, y_center);
					int adjacent_mines_other = check_remaining_mines(x, y);
					if (std::abs(adjacent_mines - adjacent_mines_other) == 0) {
						for (auto e : adjacent_covered_other) {
							if (std::find(adjacent_covered.begin(), adjacent_covered.end(), e) == adjacent_covered.end()) { // not found
								return { 0, {get<0>(e), get<1>(e)}, {x_center, y_center}, { x, y } };
							}
						}
					} else if (std::abs(adjacent_mines - adjacent_mines_other) == 1) {
						for (auto e : adjacent_covered_other) {
							if (std::find(adjacent_covered.begin(), adjacent_covered.end(), e) == adjacent_covered.end()) { // not found
								return { 1, {get<0>(e), get<1>(e)}, {x_center, y_center}, { x, y } };
							}
						}
					}
				}
				if (is_subset(adjacent_covered_other, adjacent_covered)) {
					int adjacent_mines = check_remaining_mines(x_center, y_center);
					int adjacent_mines_other = check_remaining_mines(x, y);
					if (std::abs(adjacent_mines - adjacent_mines_other) == 0) {
						for (auto e : adjacent_covered) {
							if (std::find(adjacent_covered_other.begin(), adjacent_covered_other.end(), e) == adjacent_covered_other.end()) { // not found
								return { 0, {get<0>(e), get<1>(e)}, {x_center, y_center}, { x, y } };
							}
						}
					}
					else if (std::abs(adjacent_mines - adjacent_mines_other) == 1) {
						for (auto e : adjacent_covered_other) {
							if (std::find(adjacent_covered_other.begin(), adjacent_covered_other.end(), e) == adjacent_covered_other.end()) { // not found
								return { 1, {get<0>(e), get<1>(e)}, {x_center, y_center}, { x, y } };
							}
						}
					}
				}
			}
		}
	}
	return { -1, {-1,-1}, {-1,-1}, {-1,-1} };
}

void minesweeper::MSGame::calc_p_mat()
{
	clear_p_mat();
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			int i = get_index_from_xy(x, y);
			if (get_tile_state(x, y) == states::flagged)
				p_mat[i] = 1;
			else if (get_tile_state(x, y) == states::uncovered) {
				p_mat[i] = 0;
				set_adjacent_p(x, y);
			}
		}
	}
	float avg_p = (float)remaining_mines / count_covered();
	for (int x = 0; x < columns; x++) {
		for (int y = 0; y < rows; y++) {
			int i = get_index_from_xy(x, y);
			if (get_tile_state(x, y) == states::covered && p_mat[i] < 0) {
				p_mat[i] = avg_p;
			}
		}
	}
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