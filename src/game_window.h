// Main code handling graphics, user input and interfaces with the game logic

#pragma once

#include <gtkmm.h>
#include "Minesweeper.h"
#include <map>
#include <gtk/gtk.h>

const double dark_gray = 123.0 / 255.0;
const double light_gray = 189.0 / 255.0;

class game_window : public Gtk::Window {
public:
	game_window();

protected:
	Gtk::Fixed g_fixed;
	Gtk::DrawingArea main_da;
	Gtk::DrawingArea mines_da;

	std::map<std::string, Glib::RefPtr<Gdk::Pixbuf>> g_icons;

	int sec_count = 0;
	int game_height = 16;
	int game_width = 30;
	int game_mines = 99;
	minesweeper::MSGame m_game{ 16, 30, 99 };
	sigc::connection timer_connection;

	void on_main_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
	void on_mines_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);

	void on_main_da_drag_begin(double x_offset, double y_offset);
	void on_main_da_drag_update(double x_offset, double y_offset);
	void on_main_da_drag_end(double x_offset, double y_offset);

	void on_mines_da_click_begin(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_update(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_end(Gdk::EventSequence* es, int button_num);
	void on_mines_da_click_released(int, double, double, int);
	void on_mines_da_click_unpaired_release(double, double, guint, Gdk::EventSequence*);
	bool on_mines_da_key_pressed(guint keyval, guint, Gdk::ModifierType);
	void on_mines_motion(double, double);
	void on_mines_motion();

	bool lclick_active = false;
	bool rclick_active = false;
	bool lclick_released = true;
	bool rclick_released = true;

	bool timer_handler();
	std::pair<int, int> prev_revealed{ 0,0 };
	std::pair<double, double> last_pos{ 0,0 };
	std::pair<int, int> mines_mouse_pos{ -1,-1 };

	void initialize_icons();
	void update_mine_count();
	void update_head(std::string h_string);
	void update_timer();

	enum class draw_selection{ flag, blank_uncover, re_cover, reveal };

	void redraw_cells(std::vector<std::pair<int, int>>& cells, draw_selection draw_type); 

	Cairo::RefPtr<Cairo::ImageSurface> main_da_surface;
	Cairo::RefPtr<Cairo::ImageSurface> mines_da_surface;

	Glib::RefPtr<Gtk::GestureClick> lm_click;
	Glib::RefPtr<Gtk::GestureClick> rm_click;
	Glib::RefPtr<Gtk::GestureDrag> main_drag;
	Glib::RefPtr<Gtk::EventControllerKey> key_controller;
	Glib::RefPtr<Gtk::EventControllerMotion> mines_mouse_motion;
};

void draw_line(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, int line_width, double r, double g, double b);

void draw_rect_filled(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, double r, double g, double b);