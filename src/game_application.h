// Main code handling graphics, user input and interfaces with the game logic

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
	void on_settings_window_hide();
	void on_menu_game_settings();
	void on_new_game_settings(int height, int width, int mines, int selection);
	bool on_settings_menu_close();
	bool on_app_close();

	Glib::RefPtr<Gtk::Builder> g_refBuilder;
	settings_window* s_window = nullptr;
	game_window* g_window = nullptr;
};