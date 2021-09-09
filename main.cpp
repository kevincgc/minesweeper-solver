#include "game_window.h"

int main(int argc, char* argv[]) {
	auto app = Gtk::Application::create("org.gtkmm.a.b");

	return app->make_window_and_run<game_window>(argc, argv);
}