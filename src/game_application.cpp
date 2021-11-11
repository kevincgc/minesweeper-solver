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
	add_action("game_code", sigc::mem_fun(*this, &game_application::on_menu_game_code));
	add_action("game_about", sigc::mem_fun(*this, &game_application::on_menu_game_about));
	add_action("game_step", sigc::mem_fun(*this, &game_application::on_menu_game_step));

	auto menu_object = g_refBuilder->get_object("main_menubar");
	auto main_menu_ptr = std::dynamic_pointer_cast<Gio::Menu>(menu_object);
	set_menubar(main_menu_ptr);
}

void game_application::on_activate() {
	g_window = new game_window;
	add_window(*g_window);
	g_window->set_show_menubar();
	g_window->show();
	g_window->signal_close_request().connect(sigc::mem_fun(*this, &game_application::on_app_close), false);
	g_window->signal_code_resize().connect(sigc::mem_fun(*this, &game_application::on_code_resize_game_window));
}

bool game_application::on_app_close() {

	if (gc_window)
		gc_window->close();
	if (s_window)
		s_window->close();
	if (about_window)
		about_window->close();

	return false;
}

void game_application::on_new_game_settings(int height, int width, int mines, int selection) {
	// Generate new game_window with new settings. Doing this instead of updating existing window
	// as this seems to be the only way to automatically resize the window

	bool edit_mode_active = g_window->m_game.get_game_state() == minesweeper::g_states::edit;

	remove_window(*g_window);
	if (g_window)
		delete g_window;
	g_window = new game_window(height, width, mines, selection);
	g_window->set_show_menubar();
	add_window(*g_window);
	g_window->show();
	if (edit_mode_active) {
		g_window->m_game.toggle_edit_mode(true);
		g_window->reveal_all_for_edit();
	}
	g_window->signal_close_request().connect(sigc::mem_fun(*this, &game_application::on_app_close), false);
	g_window->signal_code_resize().connect(sigc::mem_fun(*this, &game_application::on_code_resize_game_window));

	if (gc_window) {
		g_window->code_window_ptr = gc_window;
		gc_window->set_code_button_active(false);
		gc_window->generate_code_button_signal.disconnect();
		gc_window->edit_mode_button_signal.disconnect();
		gc_window->generate_code_button_signal = gc_window->edit_mode_button.signal_toggled().connect(sigc::mem_fun(*g_window, &game_window::on_edit_mode_toggle));
		gc_window->edit_mode_button_signal = gc_window->generate_code_button.signal_clicked().connect(sigc::mem_fun(*g_window, &game_window::on_generate_code_clicked));
	}
	if (s_window)
		s_window->close();
	s_window = nullptr;
}

void game_application::on_code_resize_game_window(int height, int width, int mines, std::string g_code) {

	bool edit_mode_active = g_window->m_game.get_game_state() == minesweeper::g_states::edit;

	remove_window(*g_window);
	if (g_window)
		delete g_window;
	g_window = new game_window(height, width, mines, 3);
	g_window->game_code = g_code;
	if (edit_mode_active) {
		g_window->m_game.toggle_edit_mode(true);
		g_window->reveal_all_for_edit();
	}
	g_window->set_show_menubar();
	add_window(*g_window);
	g_window->show();
	g_window->signal_close_request().connect(sigc::mem_fun(*this, &game_application::on_app_close), false);
	g_window->signal_code_resize().connect(sigc::mem_fun(*this, &game_application::on_code_resize_game_window));

	if (gc_window) {
		g_window->code_window_ptr = gc_window;
		gc_window->generate_code_button_signal.disconnect();
		gc_window->edit_mode_button_signal.disconnect();
		gc_window->generate_code_button_signal = gc_window->edit_mode_button.signal_toggled().connect(sigc::mem_fun(*g_window, &game_window::on_edit_mode_toggle));
		gc_window->edit_mode_button_signal = gc_window->generate_code_button.signal_clicked().connect(sigc::mem_fun(*g_window, &game_window::on_generate_code_clicked));
	}
}

void game_application::on_menu_game_settings() {
	if (s_window)
		s_window->close();
	s_window = new settings_window(g_window->game_height, g_window->game_width, g_window->game_mines, g_window->selection);
	s_window->signal_update_game().connect(sigc::mem_fun(*this, &game_application::on_new_game_settings));
	s_window->signal_close_request().connect(sigc::bind(sigc::mem_fun(*this, &game_application::on_signal_close_request), "s_window"), false);
	s_window->show();
}

void game_application::on_menu_game_code() {
	g_window->code_window_ptr = nullptr;
	if (gc_window)
		gc_window->close();
	gc_window = new game_code_window;
	g_window->code_window_ptr = gc_window;
	gc_window->signal_close_request().connect(sigc::mem_fun(*g_window, &game_window::on_code_window_close), false);
	gc_window->signal_close_request().connect(sigc::bind(sigc::mem_fun(*this, &game_application::on_signal_close_request), "gc_window"), false);
	if (g_window->game_code != "") {
		gc_window->set_code(g_window->game_code);
		gc_window->set_code_button_active();
	}
	else {
		if (g_window->m_game.get_game_state() != minesweeper::g_states::new_game)
			gc_window->set_code(g_window->m_game.get_game_code());
	}
	gc_window->generate_code_button_signal = gc_window->edit_mode_button.signal_toggled().connect(sigc::mem_fun(*g_window, &game_window::on_edit_mode_toggle));
	gc_window->edit_mode_button_signal = gc_window->generate_code_button.signal_clicked().connect(sigc::mem_fun(*g_window, &game_window::on_generate_code_clicked));

	if (g_window->m_game.get_game_state() == minesweeper::g_states::edit)
		gc_window->set_edit_mode_active();

	gc_window->show();
}

using Clock = std::chrono::high_resolution_clock;
int ctr = 0;
gboolean stepg(gpointer data) {
	((game_application*)data)->step();
	if (ctr == 0)
		return false;
	return true;
}

void game_application::on_menu_game_step() {
	//auto t = Clock::now();
	//int reps = 0;
	//while (reps != 10) {
	//	auto now = Clock::now();
	//	float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
	//	if (elapsed_ms > 500) {
	//		step();
	//		t = now;
	//		reps++;
	//	}
	//}
	ctr = 10;
	g_timeout_add(10, stepg, this);
	std::vector<std::pair<int, int>> v1 = { {1,1},{2,2},{2,3},{3,3},{5,4} };
	std::vector<std::pair<int, int>> v2 = { {2,3},{3,3},{2,2}, {3,2} };
	std::cout << g_window->m_game.is_subset(v1, v2) << std::endl;
	std::cout << g_window->m_game.is_subset(v2, v1) << std::endl;
}

void game_application::step() {
	//g_window->m_game.solver_step();
	g_window->m_game.calc_p_mat();
	std::vector<float> p_mat = g_window->m_game.p_mat;
	std::pair<int,int> m = g_window->m_game.find_mine();
	if (get<0>(m) != -1) {
		g_window->m_game.set_flag(std::get<0>(m), std::get<1>(m));
		std::vector <std::pair<int, int>> cells;
		cells.push_back(m);
		g_window->redraw_cells(cells, game_window::draw_selection::flag);
		std::cout << "mine at: (" << std::get<0>(m) << "," << std::get<1>(m) << ")" << std::endl;
		return;
	}
	std::pair<int, int> d = g_window->m_game.find_d_clear();
	if (get<0>(d) != -1) {
		std::vector <std::pair<int, int>> cells;
		cells = g_window->m_game.d_click_clear(std::get<0>(d), std::get<1>(d));
		g_window->redraw_cells(cells, game_window::draw_selection::reveal);
		std::cout << "d_clear at: (" << std::get<0>(d) << "," << std::get<1>(d) << ")" << std::endl;
		return;
	}
	std::pair<int, std::pair<int, int>> overlap = g_window->m_game.find_overlap();
	if (get<0>(overlap) == 0) {
		std::vector <std::pair<int, int>> cells;
		cells = g_window->m_game.l_click_clear(overlap.second.first, overlap.second.second);
		g_window->redraw_cells(cells, game_window::draw_selection::reveal);
		std::cout << "overlap_clear at: (" << overlap.second.first << "," << overlap.second.second << ")" << std::endl;
		return;
	} else if (get<0>(overlap) == 1) {
		g_window->m_game.set_flag(overlap.second.first, overlap.second.second);
		std::vector <std::pair<int, int>> cells;
		cells.push_back({ overlap.second.first, overlap.second.second });
		g_window->redraw_cells(cells, game_window::draw_selection::flag);
		std::cout << "mine at: (" << overlap.second.first << "," << overlap.second.second << ")" << std::endl;
		return;
	}
	std::vector<std::pair<int, int>> min_p_tuples = g_window->m_game.get_min_p_tuples();
	std::srand(std::time(nullptr));
	int i = std::rand() % min_p_tuples.size();
	auto cells = g_window->m_game.l_click_clear(std::get<0>(min_p_tuples[i]), std::get<1>(min_p_tuples[i]));
	g_window->redraw_cells(cells, game_window::draw_selection::reveal);
	std::cout << "l_clear at: (" << std::get<0>(min_p_tuples[i]) << "," << std::get<1>(min_p_tuples[i]) << ")" << std::endl;
}

void game_application::on_menu_game_about() {
	if (about_window)
		about_window->close();
	about_window = new game_about_window;
	about_window->signal_close_request().connect(sigc::bind(sigc::mem_fun(*this, &game_application::on_signal_close_request), "about_window"), false);

	about_window->show();
}

bool game_application::on_signal_close_request(std::string s) {
	if (s == "gc_window") {
		gc_window->close();
		gc_window = nullptr;
	}
	if (s == "about_window") {
		about_window->close();
		about_window = nullptr;
	}
	if (s == "s_window") {
		s_window->close();
		s_window = nullptr;
	}
	return false;
}