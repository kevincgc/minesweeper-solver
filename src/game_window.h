// Main code handling graphics, user input. Interfaces with the game logic

#pragma once
#include <gtkmm/fixed.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/gestureclick.h>
#include <gtkmm/gesturedrag.h>
#include <gtkmm/eventcontrollerkey.h>
#include <gtkmm/eventcontrollermotion.h>
#include <gtkmm/applicationwindow.h>
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

#include "Minesweeper.h"
#include <map>

// RGB value (same across all three channels) for dark_gray colour used in game
const double dark_gray = 123.0 / 255.0;
// RGB value (same across all three channels) for light_gray colour used in game
const double light_gray = 189.0 / 255.0;

/// <summary>
/// Game window, which is the main window for this application
/// </summary>
class game_window : public Gtk::ApplicationWindow, public sigc::trackable {

public:

	game_window();
	game_window(int height, int width, int mines, int selection);
	friend class game_application;
protected:

	Gtk::DrawingArea main_da;	// Main drawing area, encompasses entire window
	Gtk::DrawingArea mines_da;	// Mines drawing area, covers only the game tiles
	Gtk::Fixed g_fixed;			// Fixed, encompasses entire window and facillitates the overlapping DAs
	std::map<std::string, Glib::RefPtr<Gdk::Pixbuf>> g_icons;	// gresources game icons

	int sec_count = 0;		// seconds counter for time elapsed
	int game_height = 16;
	int game_width = 30;
	int game_mines = 99;
	int selection = 2;		// Easy = 0; Medium = 1; Hard = 2; Custom = 3;
	minesweeper::MSGame m_game{ 16, 30, 99 };	// default game = hard
	sigc::connection timer_connection;

	// Main draw functions for the two drawing areas
	void on_main_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
	void on_mines_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

	// Mouse input handlers for main_da
	void on_main_da_drag_begin(double x_offset, double y_offset);
	void on_main_da_drag_update(double x_offset, double y_offset);
	void on_main_da_drag_end(double x_offset, double y_offset);

	// Mouse input handlers for mines_da
	void on_mines_da_click_begin(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_update(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_end(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_released(int, double, double, int);
	void on_mines_da_click_unpaired_release(double, double, guint, Gdk::EventSequence*);
	bool on_mines_da_key_pressed(guint keyval, guint, Gdk::ModifierType);
	void on_mines_motion(double, double);
	void on_mines_motion();

	// state variables for mines_da mouse inputs
	bool lclick_active = false;
	bool rclick_active = false;
	bool lclick_released = true;
	bool rclick_released = true;
	std::pair<int, int> prev_revealed{ 0,0 };
	std::pair<double, double> last_pos{ 0,0 };
	std::pair<int, int> mines_mouse_pos{ -1,-1 };

	bool timer_handler();

	// enums to specify what type of tile to draw
	enum class draw_selection { flag, blank_uncover, re_cover, reveal };

	// helper functions for initalization, drawing, etc.
	void initialize_window();
	void initialize_icons();
	void update_mine_count();
	void update_head(std::string h_string);
	void update_timer();
	void redraw_cells(std::vector<std::pair<int, int>>& cells, draw_selection draw_type);

	// Stored ptrs to previously rendered frames - new frames edit old ones
	Cairo::RefPtr<Cairo::ImageSurface> main_da_surface;
	Cairo::RefPtr<Cairo::ImageSurface> mines_da_surface;

	// Stored ptrs to eventcontrollers
	Glib::RefPtr<Gtk::GestureClick> lm_click;
	Glib::RefPtr<Gtk::GestureClick> rm_click;
	Glib::RefPtr<Gtk::GestureDrag> main_drag;
	Glib::RefPtr<Gtk::EventControllerKey> key_controller;
	Glib::RefPtr<Gtk::EventControllerMotion> mines_mouse_motion;
};

// Wrapper for drawing a line on a cairo::context
void draw_line(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, int line_width, double r, double g, double b);

// Wrapper for drawing a filled rectangle on a cairo::context
void draw_rect_filled(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, double r, double g, double b);

/// <summary>
/// Settings window, for settings of the minesweeper game
/// </summary>
class settings_window : public Gtk::Window {

public:

	settings_window();
	settings_window(int height, int width, int mines, int selection);

	// Signal to update game with new settings
	using update_game_signal = sigc::signal<void(int, int, int, int)>;
	update_game_signal signal_update_game();

protected:

	// Widgets used in this window
	Gtk::Button apply_button;
	Gtk::CheckButton rad_buttons[4];
	Gtk::Separator box_separator;
	Gtk::Grid main_grid, top_grid, bottom_grid;
	Gtk::Entry custom_num_entries[3];
	Gtk::Label num_labels[4];
	Gtk::Label description_labels[4][3];

	// Current settings within settings menu. If apply button is activated,
	// the settings are propagated to the game window.
	int game_height = 16;
	int game_width = 30;
	int game_mines = 99;
	int selection = 2; // Easy = 0; Medium = 1; Hard = 2; Custom = 3;

	// helper function for initialization
	void initialize_settings();
	
	// text handler - forces numeric entries only
	void on_text_entry_input(int entry_num);
	// apply button handler - parses and passes the settings
	void on_click_apply_button();

	update_game_signal settings_signal;
};