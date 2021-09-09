#include "game_window.h"

game_window::game_window() {
	set_title("Basic app");
	set_default_size(200, 200);
}

//game_window::game_window(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder) :
//	Gtk::ApplicationWindow(cobject), m_refBuilder(refBuilder) {}
//
//game_window* game_window::create() {
//	auto refBuilder = Gtk::Builder::create_from_resource
//}