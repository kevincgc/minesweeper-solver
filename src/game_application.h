// Main application - handles all game windows and interactions between them

#pragma once
#include "game_window.h"
#include <thread>
#include <chrono>

class game_application : public Gtk::Application {
protected:
	game_application();

public:
	static Glib::RefPtr<game_application> create();
	void step();
	game_window* g_window = nullptr;

protected:
	void on_startup() override;
	void on_activate() override;

private:
	// handler for freeing memory before terminating application. Needed since
	// game_window and setting_window instances are dynamically initialized.
	bool on_app_close();

	// handler for opening a new settings window
	void on_menu_game_settings();

	// handler for transferring new settings to game_window
	void on_new_game_settings(int height, int width, int mines, int selection);

	// handler for game state code input/output
	void on_menu_game_code();

	// handler for about game menu
	void on_menu_game_about();
	void on_menu_game_step();
	

	// handler to resize window based on game code
	void on_code_resize_game_window(int height, int width, int mines, std::string game_code);

	bool on_signal_close_request(std::string);

	Glib::RefPtr<Gtk::Builder> g_refBuilder;
	// keep track of window instances so they can be closed (deleted) and reopened (new)

	
	game_code_window* gc_window = nullptr;
	game_about_window* about_window = nullptr;
	settings_window* s_window = nullptr;
};