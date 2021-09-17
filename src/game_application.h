// Main application - handles all game windows and interactions between them

#pragma once
#include "game_window.h"
#include <gtkmm/builder.h>
#include <gtkmm/application.h>

class game_application : public Gtk::Application {
protected:
	game_application();

public:
	static Glib::RefPtr<game_application> create();

protected:
	void on_startup() override;
	void on_activate() override;

private:
	// handler for opening a new settings window
	void on_menu_game_settings();
	// handler for transferring new settings to game_window
	void on_new_game_settings(int height, int width, int mines, int selection);
	// handler for freeing memory before terminating application. Needed since
	// game_window and setting_window instances are dynamically initialized.
	bool on_app_close();

	Glib::RefPtr<Gtk::Builder> g_refBuilder;
	// keep track of window instances so they can be closed (deleted) and reopened (new)
	settings_window* s_window = nullptr;
	game_window* g_window = nullptr;
};