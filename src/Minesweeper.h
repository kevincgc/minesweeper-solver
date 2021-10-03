// Main code for game logic

#pragma once
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <map>

#include <gtkmm/fixed.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/gesturedrag.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/eventcontrollermotion.h>
#include <gtkmm/applicationwindow.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textview.h>
#include <gdkmm/pixbuf.h>
#include <gdkmm/general.h>
#include <glibmm/main.h>
#include <gtkmm/grid.h>
#include <gtkmm/button.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <sigc++/trackable.h>
#include <sigc++/signal.h>
#include <gtkmm/builder.h>
#include <gtkmm/application.h>



#include <gtk/gtk.h>
#include <gtkmm/windowgroup.h>
#include <gtkmm/messagedialog.h>

// Helper arrays for octile distances
inline const int dx8[8]{ -1, -1, -1, 0, 0, 1, 1, 1 };
inline const int dy8[8]{ -1, 0, 1, -1, 1, -1, 0, 1 };

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

		/// <summary>
		/// Applies double-click input on tile located at (x,y).
		/// If tile is covered or flagged, the function does nothing.
		/// 
		/// If the tile is uncovered, it will check to see if it the
		/// number of flagged tiles around it equals the number of mines
		/// that are surrounding it. If true, left click inputs will be
		/// called on all surrounding tiles to reveal any unrevealed tiles.
		/// </summary>
		/// <param name="x">x-position</param>
		/// <param name="y">y-position</param>
		/// <returns>vector of std::pairs for coordinates of all
		/// tiles that changed states as a result of this action</returns>
		std::vector<std::pair<int,int>> d_click_clear(int x, int y);

		/// <summary>
		/// Applies left-click input on tile located at (x,y).
		/// If tile is uncovered or flagged, the function does nothing.
		/// 
		/// If the tile is a mine, it will trigger game lose.
		/// If the tile has no mines surrounding it, it will bfs all surrounding
		/// tiles to reveal them as well.
		/// If the tile has mines surrounding it, it will just reveal this tile.
		/// </summary>
		/// <param name="x">x-position</param>
		/// <param name="y">y-position</param>
		/// <returns>vector of std::pairs for coordinates of all
		/// tiles that changed states as a result of this action</returns>
		std::vector<std::pair<int, int>> l_click_clear(int x, int y);

		/// <summary>
		/// Helper function to left-click and right-click actions. Does
		/// DFS search to uncover all eligible tiles.
		/// </summary>
		/// <param name="u_tiles">Vector of coordinate pairs passed in by reference.
		/// It will append to the vector any new tiles that change states as a result
		/// of this function.</param>
		/// <param name="x">x-position</param>
		/// <param name="y">y-position</param>
		void reveal_adj(std::vector<std::pair<int, int>>& u_tiles, int x, int y);

		void initialize_game(std::string);

		int get_rows() { return rows; };
		int get_cols() { return columns; };
		int get_mines() { return mines; };
		int get_remaining() { return remaining_mines; };
		g_states get_game_state() { return current_state; };
		states get_tile_state(int x, int y) { return game_tiles[y * columns + x].get_state(); }
		int get_tile_type(int x, int y) { return game_tiles[y * columns + x].get_type(); }

		std::string get_game_code();

	private:
		std::vector<tile> game_tiles;
		int rows = 0;
		int columns = 0;
		int mines = 0;
		int remaining_mines = 0;
		int remaining_uncleared = 0;
		bool initialized = false;
		g_states current_state = g_states::new_game;
	};

	/// <summary>
	/// Performs Fisher-yates shuffle on elements contained within Container T.
	/// Uses std::chrono::system_clock to seed a mt19937 engine to generate
	/// random vals for swapping.
	/// 
	/// Optionally allows for shuffling # elements < size of the container. If this is selected
	/// the first shuffle_number elements of container will be randomly chosen
	/// elements. Remaining elements are not guaranteed to have random positions relative
	/// to initial positions.
	/// </summary>
	/// <typeparam name="T">Must have iterator .begin() and .end() defined.
	/// Must be MoveAssignable and MoveConstructible.</typeparam>
	/// <param name="arr">Container to be shuffled</param>
	/// <param name="shuffle_number"># of elements to shuffle. Defaults to 0; returns if less than 0;
	/// automatically set to size of arr if greater than size of arr</param>
	template <class T>
	void fy_shuffle(T& arr, int shuffle_number = 0) { // implementation of fisher yates shuffle

		if (shuffle_number < 0)
			return;

		int arr_size = static_cast<int> (arr.end() - arr.begin());

		if (shuffle_number == 0 || shuffle_number >= arr_size)
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

	bool check_code(std::string, int&, int&, int&);
}