// Main code for game logic

#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <chrono>

// Helper arrays for octile distances
const int dx8[8]{ -1, -1, -1, 0, 0, 1, 1, 1 };
const int dy8[8]{ -1, 0, 1, -1, 1, -1, 0, 1 };

namespace minesweeper {

	/// Display states for game_tiles
	enum class states { covered, uncovered, flagged };
	/// Game states
	enum class g_states { new_game, in_progress, lost, won };

	/// Tile class for storing each individual cell of the game grid
	class tile {
		friend class MSGame;
		
	public:

		tile() {}
		tile(int x, int y, int type, states tflag);

		/// Toggles state of current tile between covered/flagged.
		/// Does nothing if tile is state::uncovered.
		void flip_flag();

		/// <summary>
		/// Get type of current tile.
		/// -2 means this mine exploded;
		/// -1 means this tile is a mine;
		/// 0 means no mines next to this tile;
		/// > 0 is how many mines lie next to this tile;
		/// </summary>
		/// <returns>int between -2 to 8</returns>
		int get_type() { return tile_type; }

		/// <summary>
		/// Get state of the current tile: states::covered, states::uncovered, states::flagged
		/// </summary>
		states get_state() { return tile_state; }

	private:
		int tile_x = 0;
		int tile_y = 0;
		int tile_type = 0; // > 0 is how many mines, -1 means this is a mine, 0 means no mines; gets set to -2 if its the mine that loses the game
		states tile_state = states::covered;

	};
	class MSGame {

	public:

		/// <summary>
		/// Default constructor: sets params to "hard" game
		/// </summary>
		MSGame() : game_tiles(16*30), rows(16), columns(30), mines(99), remaining_mines(99) {}

		/// <summary>
		/// 
		/// </summary>
		/// <param name="r"># of rows</param>
		/// <param name="c"># of columns</param>
		/// <param name="m"># of mines</param>
		MSGame(int r, int c, int m) : game_tiles(r*c), rows(r), columns(c), mines(m), remaining_mines(m) {}

		void reset();
		void reset(int r, int c, int m);
		void initialize_game(int x, int y);

		int check_adjacent_mines(int x, int y);
		int check_adjacent_flags(int x, int y);
		int set_flag(int x, int y);
		void inc_remaining() { remaining_mines++; }
		void dec_remaining() { remaining_mines--; }

		std::vector<std::pair<int,int>> d_click_clear(int x, int y);
		std::vector<std::pair<int, int>> l_click_clear(int x, int y);
		void reveal_adj(std::vector<std::pair<int, int>>& u_tiles, int x, int y);

		int get_rows() { return rows; };
		int get_cols() { return columns; };
		int get_mines() { return mines; };
		int get_remaining() { return remaining_mines; };
		g_states get_game_state() { return current_state; };
		states get_tile_state(int x, int y) { return game_tiles[y * columns + x].get_state(); }
		int get_tile_type(int x, int y) { return game_tiles[y * columns + x].get_type(); }

	private:
		std::vector<tile> game_tiles;
		int rows = 0;
		int columns = 0;
		int mines = 0;
		int remaining_mines = 0;
		int remaining_uncleared = 0;
		g_states current_state = g_states::new_game;
	};

	template <class T>
	void fy_shuffle(T& arr, int shuffle_number = 0) { // implementation of fisher yates shuffle

		if (shuffle_number < 0)
			return;

		int arr_size = static_cast<int> (arr.end() - arr.begin());

		if (shuffle_number == 0 || shuffle_number == arr_size)
			shuffle_number = arr_size - 1;

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::mt19937 mgen(seed);

		for (int i = 0; i < shuffle_number; i++) {
			// re-construct udist each time to change bounds - apparently lightweight operation
			std::uniform_int_distribution<int> udist(i, (arr_size - 1));
			std::swap(arr[i], arr[udist(mgen)]);
		}

		return;

	}
}