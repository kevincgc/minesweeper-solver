#include "game_application.h"

game_application::game_application() : Gtk::Application("jw.minesweeper") {
}

Glib::RefPtr<game_application> game_application::create() {
	return Glib::make_refptr_for_instance<game_application>(new game_application());
}

void game_application::on_startup() {
	Gtk::Application::on_startup();
	
	// Add menubar to game_window
	g_refBuilder = Gtk::Builder::create();
	g_refBuilder->add_from_resource("/ui/menu_bar.ui");

	add_action("game_settings_edit", sigc::mem_fun(*this, &game_application::on_menu_game_settings));

	auto menu_object = g_refBuilder->get_object("main_menubar");
	auto main_menu_ptr = std::dynamic_pointer_cast<Gio::Menu>(menu_object);
	set_menubar(main_menu_ptr);
}

void game_application::on_activate() {
	g_window = new game_window();
	add_window(*g_window);

	g_window->set_show_menubar();
	g_window->show();
	g_window->signal_close_request().connect(sigc::mem_fun(*this, &game_application::on_app_close), false);
}

bool game_application::on_app_close() {
	delete g_window;
	delete s_window;
	return false;
}

void game_application::on_new_game_settings(int height, int width, int mines, int selection) {
	// Generate new game_window with new settings. Doing this instead of updating existing window
	// as this seems to be the only way to automatically resize the window
	delete g_window;
	g_window = new game_window(height, width, mines, selection);
	add_window(*g_window);
	g_window->set_show_menubar();
	g_window->show();
}

void game_application::on_menu_game_settings() {
	delete s_window;
	s_window = new settings_window(g_window->game_height, g_window->game_width, g_window->game_mines, g_window->selection);
	add_window(*s_window);
	s_window->signal_update_game().connect(sigc::mem_fun(*this, &game_application::on_new_game_settings));
	s_window->show();
}