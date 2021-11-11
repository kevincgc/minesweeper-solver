#include "game_window.h"

game_window::game_window() {
	game_window::initialize_window();
}

game_window::game_window(int new_h, int new_w, int new_m, int new_selection) {

	game_height = new_h;
	game_width = new_w;
	game_mines = new_m;
	selection = new_selection;

	m_game.reset(new_h, new_w, new_m);

	game_window::initialize_window();
}

void game_window::initialize_window() {
	//Base initialization
	set_title("Minesweeper");
	initialize_icons();
	set_size_request(1, 1);
	g_fixed.set_size_request(40 + 32 * game_width, 124 + 32 * game_height);
	set_child(g_fixed);

	//Main Drawingarea
	main_da.set_content_width(40 + 32 * game_width);
	main_da.set_content_height(124 + 32 * game_height);
	g_fixed.put(main_da, 0, 0);

	//Mines Drawingarea
	mines_da.set_content_width(32 * m_game.get_cols());
	mines_da.set_content_height(32 * m_game.get_rows());
	g_fixed.put(mines_da, 20, 104);

	//Draw Functions
	main_da.set_draw_func(sigc::mem_fun(*this, &game_window::on_main_da_draw));
	mines_da.set_draw_func(sigc::mem_fun(*this, &game_window::on_mines_da_draw));

	//Connect Signals
	//--drag for main drawingarea - only applies to the reset button
	main_drag = Gtk::GestureDrag::create();
	main_drag->set_button(GDK_BUTTON_PRIMARY);
	main_da.add_controller(main_drag);
	main_drag->signal_drag_begin().connect(sigc::mem_fun(*this, &game_window::on_main_da_drag_begin));
	main_drag->signal_drag_update().connect(sigc::mem_fun(*this, &game_window::on_main_da_drag_update));
	main_drag->signal_drag_end().connect(sigc::mem_fun(*this, &game_window::on_main_da_drag_end));

	//--keyboard events for mines da - processes spacebar inputs
	key_controller = Gtk::EventControllerKey::create();
	add_controller(key_controller);
	key_controller->signal_key_pressed().connect(sigc::mem_fun(*this, &game_window::on_mines_da_key_pressed), false);

	//--mouse motion events for mines da - tracks mouse position for spacebar inputs
	mines_mouse_motion = Gtk::EventControllerMotion::create();
	mines_da.add_controller(mines_mouse_motion);
	mines_mouse_motion->signal_enter().connect(mem_fun(*this, static_cast<void (game_window::*)(double, double)>(&game_window::on_mines_motion)));
	mines_mouse_motion->signal_motion().connect(mem_fun(*this, static_cast<void (game_window::*)(double, double)>(&game_window::on_mines_motion)));
	mines_mouse_motion->signal_leave().connect(mem_fun(*this, static_cast<void (game_window::*)()>(&game_window::on_mines_motion)));

	//--gestureclicks for mines da
	lm_click = Gtk::GestureClick::create();
	lm_click->set_button(1);
	mines_da.add_controller(lm_click);
	rm_click = Gtk::GestureClick::create();
	rm_click->set_button(3);
	mines_da.add_controller(rm_click);

	/*
	There are three types of mouse events to be processed for mines_da: single left click, single right click,
	(left and right) click.

	-signal_begin fires when a button is pressed.
	-signal_update fires when the mouse moves while the signal for the button is still active
	-signal_end fires when the signal for the button is inactivated
	-signal_released fires when the mouse button is released while the signal for it is still active
	-signal_unpaired_release fires when the mouse button is released when the signal for it is inactive

	Because of how the eventcontroller for GestureClick is written, only one signal can be active at a time, which
	complicates the logic for (left and right) clicks. If a button is pressed and kept pressed while a second button is pressed,
	the first signal will end and fire signal_end and then fire a signal_begin for the second button.

	Eventhandlers below use a combination of the states of the signals (active, inactive, released, not released) to determine
	how many buttons are currently depressed and which input to process.

	*/
	lm_click->signal_begin().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_begin), 1));
	rm_click->signal_begin().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_begin), 3));

	lm_click->signal_update().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_update), 1));
	rm_click->signal_update().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_update), 3));

	lm_click->signal_end().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_end), 1));
	rm_click->signal_end().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_end), 3));

	lm_click->signal_released().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_released), 1));
	rm_click->signal_released().connect(sigc::bind(sigc::mem_fun(*this, &game_window::on_mines_da_click_released), 3));

	lm_click->signal_unpaired_release().connect(sigc::mem_fun(*this, &game_window::on_mines_da_click_unpaired_release));
	rm_click->signal_unpaired_release().connect(sigc::mem_fun(*this, &game_window::on_mines_da_click_unpaired_release));
}

void game_window::initialize_icons() {// Load icons from gresources for use
	for (int i = 0; i < 10; i++) {
		g_icons[(std::string)("c_" + std::to_string(i))] = Gdk::Pixbuf::create_from_resource("/counter/c_" + std::to_string(i) + ".png");
	}
	g_icons["c_neg"] = Gdk::Pixbuf::create_from_resource("/counter/c_neg.png");

	g_icons["ok_head"] = Gdk::Pixbuf::create_from_resource("/headicons/ok_head.png");
	g_icons["lost_head"] = Gdk::Pixbuf::create_from_resource("/headicons/lost_head.png");
	g_icons["clicked_head"] = Gdk::Pixbuf::create_from_resource("/headicons/clicked_head.png");
	g_icons["won_head"] = Gdk::Pixbuf::create_from_resource("/headicons/won_head.png");
	g_icons["reset_head"] = Gdk::Pixbuf::create_from_resource("/headicons/reset_head.png");
	g_icons["awoo_head"] = Gdk::Pixbuf::create_from_resource("/headicons/a.png");
	g_icons["awoo2_head"] = Gdk::Pixbuf::create_from_resource("/headicons/a2.png");

	for (int i = 0; i < 9; i++) {
		g_icons[(std::string)("m_" + std::to_string(i))] = Gdk::Pixbuf::create_from_resource("/mines/m_" + std::to_string(i) + ".png");
	}

	g_icons["incorrect"] = Gdk::Pixbuf::create_from_resource("/mines/incorrect.png");
	g_icons["exploded"] = Gdk::Pixbuf::create_from_resource("/mines/exploded.png");
	g_icons["flagged"] = Gdk::Pixbuf::create_from_resource("/mines/flagged.png");
	g_icons["unchecked"] = Gdk::Pixbuf::create_from_resource("/mines/unchecked.png");
	g_icons["mine"] = Gdk::Pixbuf::create_from_resource("/mines/mine.png");
}

void game_window::on_main_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {

	if (!main_da_surface) {	// If main_da_surface==NULL, nothing has been drawn yet, so intialize
		main_da_surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
		auto new_cr = Cairo::Context::create(main_da_surface);

		//draw bottom border, dark grey
		draw_line(new_cr, 2, height - 2, width, 0, 4, dark_gray, dark_gray, dark_gray);

		//draw right border, dark grey
		draw_line(new_cr, width - 2, 2, 0, height, 4, dark_gray, dark_gray, dark_gray);

		//draw bottom left corner
		draw_rect_filled(new_cr, 2, height - 4, 2, 2, light_gray, light_gray, light_gray);
		draw_rect_filled(new_cr, 0, height - 2, 2, 2, light_gray, light_gray, light_gray);

		//draw top right corner
		draw_rect_filled(new_cr, width - 4, 2, 2, 2, light_gray, light_gray, light_gray);
		draw_rect_filled(new_cr, width - 2, 0, 2, 2, light_gray, light_gray, light_gray);

		//fill main box
		draw_rect_filled(new_cr, 4, 4, width - 8, height - 8, light_gray, light_gray, light_gray);

		//top sub box
		draw_rect_filled(new_cr, 16, 16, width - (16 + 18), 4, dark_gray, dark_gray, dark_gray);
		draw_rect_filled(new_cr, 16, 16, 4, 69, dark_gray, dark_gray, dark_gray);
		draw_rect_filled(new_cr, 18, 84, width - (16 + 18), 4, 1, 1, 1);
		draw_rect_filled(new_cr, width - (16 + 4), 18, 4, 69, 1, 1, 1);
		draw_rect_filled(new_cr, 18, 84, 2, 2, light_gray, light_gray, light_gray);
		draw_rect_filled(new_cr, width - (16 + 4), 18, 2, 2, light_gray, light_gray, light_gray);

		//bottom sub box
		draw_rect_filled(new_cr, 16, 100, width - (16 + 18), 4, dark_gray, dark_gray, dark_gray);
		draw_rect_filled(new_cr, 16, 100, 4, height - (18 + 100), dark_gray, dark_gray, dark_gray);
		draw_rect_filled(new_cr, 18, height - 20, width - (16 + 18), 4, 1, 1, 1);
		draw_rect_filled(new_cr, width - (16 + 4), 100 + 2, 4, height - (18 + 100), 1, 1, 1);
		draw_rect_filled(new_cr, 18, height - 20, 2, 2, light_gray, light_gray, light_gray);
		draw_rect_filled(new_cr, width - (16 + 4), 100 + 2, 2, 2, light_gray, light_gray, light_gray);

		//draw head icon
		Gdk::Cairo::set_source_pixbuf(new_cr, g_icons["ok_head"], (width - 52) / 2, 26);
		new_cr->paint();

		update_timer();
		update_mine_count();
	}

	if (!cr)
		return;
	cr->set_source(main_da_surface, 0, 0);
	cr->paint();
}

void game_window::on_mines_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
	if (!mines_da_surface) {	// If mines_da_surface==NULL, nothing has been drawn yet, so intialize
		mines_da_surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
		auto new_cr = Cairo::Context::create(mines_da_surface);

		// Draw a grid of covered game tiles
		for (int i = 0; i < m_game.get_rows(); i++) {
			for (int j = 0; j < m_game.get_cols(); j++) {
				Gdk::Cairo::set_source_pixbuf(new_cr, g_icons["unchecked"], 32 * j, 32 * i);
				new_cr->paint();
			}
		}
	}

	if (!cr)
		return;
	cr->set_source(mines_da_surface, 0, 0);
	cr->paint();
}

bool game_window::timer_handler() {
	if (game_window::sec_count == 999)
		return false;

	++game_window::sec_count;
	game_window::update_timer();
	return true;
}

void game_window::on_main_da_drag_begin(double x_offset, double y_offset) {
	if (x_offset >= (main_da.get_width() - 52) / 2 && x_offset <= (main_da.get_width() + 52) / 2
		&& y_offset >= 26 && y_offset <= 77) {

		update_head("reset_head");
	}
}

void game_window::on_main_da_drag_update(double x_offset, double y_offset) {
	double start_x, start_y;
	main_drag->get_start_point(start_x, start_y);

	if (start_x >= (main_da.get_width() - 52) / 2 && start_x <= (main_da.get_width() + 52) / 2
		&& start_y >= 26 && start_y <= 77) {

		if (x_offset + start_x < (main_da.get_width() - 52) / 2 || x_offset + start_x >(main_da.get_width() + 52) / 2
			|| y_offset + start_y < 26 || y_offset + start_y > 77) {

			switch (m_game.get_game_state()) {
			case minesweeper::g_states::lost:
				update_head("lost_head");
				break;
			case minesweeper::g_states::won:
				update_head("won_head");
				break;
			default:
				update_head("ok_head");
			}
		}
		else {
			update_head("reset_head");
		}
	}
}

void game_window::on_main_da_drag_end(double x_offset, double y_offset) {
	double start_x, start_y;
	main_drag->get_start_point(start_x, start_y);

	if (start_x >= (main_da.get_width() - 52) / 2 && start_x <= (main_da.get_width() + 52) / 2
		&& start_y >= 26 && start_y <= 77) {

		if (x_offset + start_x >= (main_da.get_width() - 52) / 2 && x_offset + start_x <= (main_da.get_width() + 52) / 2
			&& y_offset + start_y >= 26 && y_offset + start_y <= 77) {

			bool use_code = false;

			if ((code_window_ptr && code_window_ptr->code_button_active()) || (!code_window_ptr && game_code != "")) {
				if (code_window_ptr && code_window_ptr->code_button_active())
					game_code = code_window_ptr->get_code();

				int new_r = 0, new_c = 0, new_m = 0;
				if (minesweeper::check_code(game_code, new_r, new_c, new_m)) {
					if (!(new_r == game_height && new_c == game_width)) {
						code_resize_sig.emit(new_r, new_c, new_m, game_code);
						return;
					}
					use_code = true;
				}
				else {
					game_code = "invalid code";
					if (code_window_ptr)
						code_window_ptr->set_code("invalid code");
				}
			}

			m_game.reset();
			timer_connection.disconnect();
			sec_count = 0;

			if (m_game.get_game_state() == minesweeper::g_states::edit) {
				if (use_code)
					m_game.initialize_game(game_code);
				reveal_all_for_edit();
				update_head("ok_head");
			}
			else {
				mines_da_surface = nullptr;
				mines_da.queue_draw();
				main_da_surface = nullptr;
				main_da.queue_draw();
			}
		}
	}
}

void game_window::on_mines_da_click_begin(Gdk::EventSequence* es, int button_num) {

	double pix_x, pix_y;
	if (button_num == 1)
		lm_click->get_point(es, pix_x, pix_y);
	else if (button_num == 3)
		rm_click->get_point(es, pix_x, pix_y);
	last_pos.first = pix_x;
	last_pos.second = pix_y;

	if (m_game.get_game_state() == minesweeper::g_states::lost ||
		m_game.get_game_state() == minesweeper::g_states::won ||
		m_game.get_game_state() == minesweeper::g_states::edit ||
		m_game.get_game_state() == minesweeper::g_states::editted)
		return;

	if (button_num == 1) {
		int grid_x = pix_x / 32, grid_y = pix_y / 32;

		lclick_active = true;
		lclick_released = false;
		std::vector <std::pair<int, int>> cells;

		if (!rclick_released) { // 2key press = highlight everything around
			prev_revealed = { grid_x, grid_y };
			cells.push_back({ grid_x, grid_y });

			for (int i = 0; i < 8; i++) {
				if (grid_x + dx8[i] >= 0 && grid_x + dx8[i] < m_game.get_cols()
					&& grid_y + dy8[i] >= 0 && grid_y + dy8[i] < m_game.get_rows()) {
					cells.push_back({ grid_x + dx8[i], grid_y + dy8[i] });
				}
			}

			redraw_cells(cells, game_window::draw_selection::blank_uncover);
			update_head("clicked_head");
		}
		else { // single key press = highlight current cell
			prev_revealed = { grid_x, grid_y };
			cells.push_back({ grid_x, grid_y });
			redraw_cells(cells, game_window::draw_selection::blank_uncover);
			update_head("clicked_head");
		}
	}
	else if (button_num == 3) {
		if (!rclick_released)
			return;

		int grid_x = pix_x / 32, grid_y = pix_y / 32;

		rclick_active = true;
		rclick_released = false;
		std::vector <std::pair<int, int>> cells;

		if (!lclick_released) { // 2key press = highlight everything around
			prev_revealed = { grid_x, grid_y };
			cells.push_back({ grid_x, grid_y });

			for (int i = 0; i < 8; i++) {
				if (grid_x + dx8[i] >= 0 && grid_x + dx8[i] < m_game.get_cols()
					&& grid_y + dy8[i] >= 0 && grid_y + dy8[i] < m_game.get_rows()) {
					cells.push_back({ grid_x + dx8[i], grid_y + dy8[i] });
				}
			}

			redraw_cells(cells, game_window::draw_selection::blank_uncover);
			update_head("clicked_head");
		}
		else { // single key press = flag current cell
			cells.push_back({ grid_x, grid_y });
			m_game.set_flag(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::flag);
		}
	}
}

void game_window::on_mines_da_click_update(Gdk::EventSequence* es, int button_num) {

	double pix_x, pix_y;
	if (button_num == 1)
		lm_click->get_point(es, pix_x, pix_y);
	else if (button_num == 3)
		rm_click->get_point(es, pix_x, pix_y);
	last_pos.first = pix_x;
	last_pos.second = pix_y;

	if (m_game.get_game_state() == minesweeper::g_states::lost ||
		m_game.get_game_state() == minesweeper::g_states::won ||
		m_game.get_game_state() == minesweeper::g_states::edit ||
		m_game.get_game_state() == minesweeper::g_states::editted)
		return;

	int grid_x = pix_x / 32, grid_y = pix_y / 32;

	// cover up the previously revealed squares
	if (prev_revealed.first != grid_x || prev_revealed.second != grid_y) {

		if (prev_revealed.first >= 0 && prev_revealed.second >= 0 && prev_revealed.first < m_game.get_cols() && prev_revealed.second < m_game.get_rows()) {
			if (!lclick_released && !rclick_released) {
				std::vector<std::pair<int, int>> cells;
				cells.push_back({ prev_revealed.first, prev_revealed.second });
				for (int i = 0; i < 8; i++) {
					if (prev_revealed.first + dx8[i] < 0 || prev_revealed.second + dy8[i] < 0 || prev_revealed.first + dx8[i] >= m_game.get_cols() || prev_revealed.second + dy8[i] >= m_game.get_rows())
						continue;

					cells.push_back({ prev_revealed.first + dx8[i],prev_revealed.second + dy8[i] });
				}
				redraw_cells(cells, game_window::draw_selection::re_cover);
			}
			else if (lclick_active) {
				std::vector<std::pair<int, int>> cells;
				cells.push_back({ prev_revealed.first, prev_revealed.second });
				redraw_cells(cells, game_window::draw_selection::re_cover);
			}
		}
	}

	prev_revealed.first = grid_x;
	prev_revealed.second = grid_y;

	if (pix_x >= mines_da.get_width() || pix_y >= mines_da.get_height() || pix_x < 0 || pix_y < 0)
		return;

	// uncover current squares
	if (!lclick_released && !rclick_released) {
		std::vector<std::pair<int, int>> cells;
		cells.push_back({ grid_x, grid_y });
		for (int i = 0; i < 8; i++) {
			if (grid_x + dx8[i] < 0 || grid_y + dy8[i] < 0 || grid_x + dx8[i] >= m_game.get_cols() || grid_y + dy8[i] >= m_game.get_rows())
				continue;

			cells.push_back({ grid_x + dx8[i],grid_y + dy8[i] });
		}
		redraw_cells(cells, game_window::draw_selection::blank_uncover);
	}
	else if (lclick_active) {
		std::vector<std::pair<int, int>> cells;
		cells.push_back({ grid_x, grid_y });
		redraw_cells(cells, game_window::draw_selection::blank_uncover);
	}
}

void game_window::on_mines_da_click_end(Gdk::EventSequence* es, int button_num) {

	if (m_game.get_game_state() == minesweeper::g_states::lost ||
		m_game.get_game_state() == minesweeper::g_states::won ||
		m_game.get_game_state() == minesweeper::g_states::editted)
		return;

	if (button_num == 1) {
		lclick_active = false;
		if (rclick_active && !lclick_released)
			return;
		double pix_x = last_pos.first, pix_y = last_pos.second;
		if (pix_x >= mines_da.get_width() || pix_y >= mines_da.get_height() || pix_x < 0 || pix_y < 0)
			return;

		int grid_x = pix_x / 32, grid_y = pix_y / 32;

		if ((lclick_released && !rclick_released) || !lclick_released) { // two button click
			// re-cover the cells first
			std::vector<std::pair<int, int>> cells;
			cells.push_back({ grid_x, grid_y });
			for (int i = 0; i < 8; i++) {
				if (grid_x + dx8[i] < 0 || grid_y + dy8[i] < 0 || grid_x + dx8[i] >= m_game.get_cols() || grid_y + dy8[i] >= m_game.get_rows())
					continue;

				cells.push_back({ grid_x + dx8[i], grid_y + dy8[i] });
			}
			redraw_cells(cells, game_window::draw_selection::re_cover);

			cells = m_game.d_click_clear(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::reveal);
		}
		else { // single cell reveal
			if (pix_x >= mines_da.get_width() || pix_y >= mines_da.get_height() || pix_x < 0 || pix_y < 0)
				return;

			if (m_game.get_game_state() == minesweeper::g_states::new_game && m_game.get_tile_state(grid_x, grid_y) == minesweeper::states::covered) {
				timer_connection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &game_window::timer_handler), 1);

				if ((code_window_ptr && code_window_ptr->code_button_active()) || (!code_window_ptr && game_code != "")) {
					if (code_window_ptr && code_window_ptr->code_button_active())
						game_code = code_window_ptr->get_code();

					int new_r = 0, new_c = 0, new_m = 0;
					if (minesweeper::check_code(game_code, new_r, new_c, new_m)) {
						if (new_r == game_height && new_c == game_width) {
							m_game.initialize_game(game_code);
							selection = 3;
							auto cells = m_game.l_click_clear(grid_x, grid_y);
							redraw_cells(cells, game_window::draw_selection::reveal);
						}
						else {
							code_resize_sig.emit(new_r, new_c, new_m, game_code);
							return;
						}
					}
					else {
						auto cells = m_game.l_click_clear(grid_x, grid_y);
						redraw_cells(cells, game_window::draw_selection::reveal);
						game_code = "invalid code";
						if (code_window_ptr)
							code_window_ptr->set_code("invalid code");
					}
				}
				else {
					auto cells = m_game.l_click_clear(grid_x, grid_y);
					redraw_cells(cells, game_window::draw_selection::reveal);
					if (code_window_ptr)
						code_window_ptr->set_code(m_game.get_game_code());
				}
			}
			else {
				auto cells = m_game.l_click_clear(grid_x, grid_y);
				redraw_cells(cells, game_window::draw_selection::reveal);
				if (m_game.get_game_state() == minesweeper::g_states::edit)
					update_mine_count();
			}
		}
	}
	else if (button_num == 3) {
		rclick_active = false;
		if (lclick_active && !rclick_released)
			return;
		double pix_x = last_pos.first, pix_y = last_pos.second;
		if (pix_x >= mines_da.get_width() || pix_y >= mines_da.get_height() || pix_x < 0 || pix_y < 0)
			return;
		int grid_x = pix_x / 32, grid_y = pix_y / 32;

		// two button click; nothing required if single right button click
		if ((rclick_released && !lclick_released) || !rclick_released) {
			// re-cover the cells first
			std::vector<std::pair<int, int>> cells;
			cells.push_back({ grid_x, grid_y });
			for (int i = 0; i < 8; i++) {
				if (grid_x + dx8[i] < 0 || grid_y + dy8[i] < 0 || grid_x + dx8[i] >= m_game.get_cols() || grid_y + dy8[i] >= m_game.get_rows())
					continue;

				cells.push_back({ grid_x + dx8[i], grid_y + dy8[i] });
			}
			redraw_cells(cells, game_window::draw_selection::re_cover);

			cells = m_game.d_click_clear(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::reveal);
		}
	}

	update_head("ok_head");

	if (m_game.get_game_state() == minesweeper::g_states::lost) {
		timer_connection.disconnect();//do lose stuff here;
		update_head("lost_head");

		auto cr = Cairo::Context::create(mines_da_surface);

		for (int x = 0; x < m_game.get_cols(); x++) {
			for (int y = 0; y < m_game.get_rows(); y++) {
				int tile_type = m_game.get_tile_type(x, y);
				if (tile_type >= 0 && m_game.get_tile_state(x, y) != minesweeper::states::flagged)
					continue;

				if (tile_type == -2) {
					Gdk::Cairo::set_source_pixbuf(cr, g_icons["exploded"], 32 * x, 32 * y);
				}
				else {
					if (m_game.get_tile_state(x, y) == minesweeper::states::flagged) {
						if (tile_type == -1)
							Gdk::Cairo::set_source_pixbuf(cr, g_icons["flagged"], 32 * x, 32 * y);
						else
							Gdk::Cairo::set_source_pixbuf(cr, g_icons["incorrect"], 32 * x, 32 * y);
					}
					else
						Gdk::Cairo::set_source_pixbuf(cr, g_icons["mine"], 32 * x, 32 * y);
				}
				cr->paint();
			}
		}
	}
	if (m_game.get_game_state() == minesweeper::g_states::won) {
		timer_connection.disconnect();
		update_head("won_head");
	}

	mines_da.queue_draw();
	main_da.queue_draw();
}

void game_window::on_mines_da_click_released(int, double, double, int button_num) {
	switch (button_num) {
	case(1): lclick_released = true; break;
	case(3): rclick_released = true; break;
	}
}

void game_window::on_mines_da_click_unpaired_release(double, double, guint button_num, Gdk::EventSequence* es) {
	switch (button_num) {
	case(1): lclick_released = true; break;
	case(3): rclick_released = true; break;
	}

	if (lclick_released && rclick_released && (m_game.get_game_state() == minesweeper::g_states::in_progress || m_game.get_game_state() == minesweeper::g_states::new_game))
		update_head("ok_head");
}

void game_window::on_mines_motion(double x, double y) {
	mines_mouse_pos.first = x;
	mines_mouse_pos.second = y;
}

void game_window::on_mines_motion() {	// If no params passed, this was called by mouse leaving the box
	mines_mouse_pos.first = -1;
	mines_mouse_pos.second = -1;
}

bool game_window::on_mines_da_key_pressed(guint keyval, guint, Gdk::ModifierType) {

	if (keyval == 32) {
		if (m_game.get_game_state() == minesweeper::g_states::lost ||
			m_game.get_game_state() == minesweeper::g_states::won ||
			m_game.get_game_state() == minesweeper::g_states::editted)
			return false;

		if (mines_mouse_pos.first == -1)
			return false;
		std::vector <std::pair<int, int>> cells;

		int grid_x = mines_mouse_pos.first / 32, grid_y = mines_mouse_pos.second / 32;

		if (m_game.get_game_state() == minesweeper::g_states::edit) {
			cells = m_game.l_click_clear(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::reveal);
			update_mine_count();
			return false;
		}

		if (m_game.get_tile_state(grid_x, grid_y) == minesweeper::states::uncovered) {
			cells = m_game.d_click_clear(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::reveal);
		}
		else { // either flagged or unflagged, redraw and set_flag will flip it
			cells.push_back({ grid_x, grid_y });
			m_game.set_flag(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::flag);
		}

		if (m_game.get_game_state() == minesweeper::g_states::lost) {
			timer_connection.disconnect();//do lose stuff here;
			update_head("lost_head");

			auto cr = Cairo::Context::create(mines_da_surface);

			for (int x = 0; x < m_game.get_cols(); x++) {
				for (int y = 0; y < m_game.get_rows(); y++) {
					int tile_type = m_game.get_tile_type(x, y);
					if (tile_type >= 0 && m_game.get_tile_state(x, y) != minesweeper::states::flagged)
						continue;

					if (tile_type == -2) {
						Gdk::Cairo::set_source_pixbuf(cr, g_icons["exploded"], 32 * x, 32 * y);
					}
					else {
						if (m_game.get_tile_state(x, y) == minesweeper::states::flagged) {
							if (tile_type == -1)
								Gdk::Cairo::set_source_pixbuf(cr, g_icons["flagged"], 32 * x, 32 * y);
							else
								Gdk::Cairo::set_source_pixbuf(cr, g_icons["incorrect"], 32 * x, 32 * y);
						}
						else
							Gdk::Cairo::set_source_pixbuf(cr, g_icons["mine"], 32 * x, 32 * y);
					}
					cr->paint();
				}
			}
		}
		if (m_game.get_game_state() == minesweeper::g_states::won) {
			timer_connection.disconnect();
			update_head("won_head");
		}

		mines_da.queue_draw();
		main_da.queue_draw();
	}

	return false;
}

void game_window::redraw_cells(std::vector<std::pair<int, int>>& cells, game_window::draw_selection draw_type) {

	if (cells.empty())
		return;

	auto cr = Cairo::Context::create(mines_da_surface);

	if (draw_type == game_window::draw_selection::flag) {
		for (auto& cell : cells) {
			if (m_game.get_tile_state(cell.first, cell.second) == minesweeper::states::flagged) {
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["flagged"], cell.first * 32, cell.second * 32);
				cr->paint();
			}
			else if (m_game.get_tile_state(cell.first, cell.second) == minesweeper::states::covered) {
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["unchecked"], cell.first * 32, cell.second * 32);
				cr->paint();
			}
		}
		game_window::update_mine_count();
	}
	else if (draw_type == game_window::draw_selection::blank_uncover) {
		for (auto& cell : cells) {
			if (m_game.get_tile_state(cell.first, cell.second) != minesweeper::states::covered)
				continue;

			Gdk::Cairo::set_source_pixbuf(cr, g_icons["m_0"], cell.first * 32, cell.second * 32);
			cr->paint();
		}
	}
	else if (draw_type == game_window::draw_selection::re_cover) {
		for (auto& cell : cells) {
			if (m_game.get_tile_state(cell.first, cell.second) != minesweeper::states::covered)
				continue;

			Gdk::Cairo::set_source_pixbuf(cr, g_icons["unchecked"], cell.first * 32, cell.second * 32);
			cr->paint();
		}
	}
	else if (draw_type == game_window::draw_selection::reveal) {
		for (auto& cell : cells) {
			int num = m_game.get_tile_type(cell.first, cell.second);

			if (num < 0)
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["mine"], cell.first * 32, cell.second * 32);
			else
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["m_" + std::to_string(num)], cell.first * 32, cell.second * 32);

			cr->paint();
		}
	}

	mines_da.queue_draw();

}

void game_window::update_mine_count() {

	int x_offset = 32;
	int icon_width = 26;
	int y_offset = 28;

	auto cr = Cairo::Context::create(main_da_surface);
	int mines = m_game.get_remaining();

	int digits[3];
	if (mines >= 0) {
		for (int i = 2; i >= 0; i--) {
			digits[i] = mines % 10;
			mines /= 10;
		}

		for (int i = 0; i < 3; i++) {
			Gdk::Cairo::set_source_pixbuf(cr, g_icons["c_" + std::to_string(digits[i])], x_offset + i * icon_width, y_offset);
			cr->paint();
		}
	}
	else {
		mines *= -1;
		for (int i = 2; i >= 0; i--) {
			digits[i] = mines % 10;
			mines /= 10;
		}

		Gdk::Cairo::set_source_pixbuf(cr, g_icons["c_neg"], x_offset, y_offset);
		cr->paint();

		for (int i = 1; i < 3; i++) {
			Gdk::Cairo::set_source_pixbuf(cr, g_icons["c_" + std::to_string(digits[i])], x_offset + i * icon_width, y_offset);
			cr->paint();
		}
	}
	main_da.queue_draw();
}

void game_window::update_timer() {
	int digits[3];
	int temp_count = game_window::sec_count;

	for (int i = 2; i >= 0; i--) {
		digits[i] = temp_count % 10;
		temp_count /= 10;
	}

	auto cr = Cairo::Context::create(main_da_surface);

	for (int i = 0; i < 3; i++) {
		Gdk::Cairo::set_source_pixbuf(cr, g_icons["c_" + std::to_string(digits[i])], main_da.get_width() - 110 + 26 * i, 28);
		cr->paint();
	}

	main_da.queue_draw();
}

void game_window::update_head(std::string h_string) {
	auto cr = Cairo::Context::create(main_da_surface);
	if (!g_icons.count(h_string))
		throw std::invalid_argument("head icon doesn't exist");
	Gdk::Cairo::set_source_pixbuf(cr, g_icons[h_string], (main_da.get_width() - 52) / 2, 26);
	cr->paint();

	main_da.queue_draw();
}

bool game_window::on_code_window_close() {
	if (!code_window_ptr)
		return false;
	if (code_window_ptr->code_button_active())
		game_code = code_window_ptr->get_code();
	else
		game_code = "";
	return false;
}

void game_window::on_edit_mode_toggle() {
	if (!code_window_ptr)
		return;

	auto curr_state = m_game.get_game_state();

	m_game.toggle_edit_mode(code_window_ptr->edit_mode_active());
	if (code_window_ptr->edit_mode_active()) {
		timer_connection.disconnect();

		// If game is new_game state prior to setting to edit reset the game so that mine count is set to 0.
		if (curr_state == minesweeper::g_states::new_game)
			m_game.reset();

		reveal_all_for_edit();
	}
}

void game_window::on_generate_code_clicked() {
	if (!code_window_ptr)
		return;

	if (code_window_ptr->code_button_active())
		game_code = m_game.get_game_code();

	code_window_ptr->set_code(m_game.get_game_code());
}

void game_window::reveal_all_for_edit() {
	if (!mines_da_surface)
		on_mines_da_draw(nullptr, mines_da.get_content_width(), mines_da.get_content_height());
	if (!main_da_surface)
		on_main_da_draw(nullptr, main_da.get_content_width(), main_da.get_content_height());
	auto cr = Cairo::Context::create(mines_da_surface);

	for (int x = 0; x < m_game.get_cols(); x++) {
		for (int y = 0; y < m_game.get_rows(); y++) {
			int tile_type = m_game.get_tile_type(x, y);
			if (tile_type < 0)
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["mine"], 32 * x, 32 * y);
			else {
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["m_" + std::to_string(tile_type)], 32 * x, 32 * y);
			}
			cr->paint();
		}
	}
	mines_da.queue_draw();
	update_mine_count();
}

game_window::code_resize_signal game_window::signal_code_resize() {
	return code_resize_sig;
}

void draw_line(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, int line_width, double r, double g, double b) {
	cr->set_line_width(line_width);
	cr->set_source_rgb(r, g, b);
	cr->move_to(x, y);
	cr->rel_line_to(dx, dy);
	cr->stroke();
}

void draw_rect_filled(const Cairo::RefPtr<Cairo::Context>& cr, int x, int y, int dx, int dy, double r, double g, double b) {
	cr->set_source_rgb(r, g, b);
	cr->rectangle(x, y, dx, dy);
	cr->fill();
}

settings_window::settings_window() {
	initialize_settings();
}

settings_window::settings_window(int height, int width, int mines, int sel) {
	game_height = height;
	game_width = width;
	game_mines = mines;
	selection = sel;

	initialize_settings();
}

void settings_window::initialize_settings() {
	// Base initialization
	set_title("Game Settings");
	set_child(main_grid);
	apply_button = Gtk::Button("New Game");
	set_default_widget(apply_button);
	set_resizable(false);

	num_labels[0].set_markup("<b>Easy</b>");
	num_labels[1].set_markup("<b>Medium</b>");
	num_labels[2].set_markup("<b>Hard</b>");
	num_labels[3].set_markup("<b>Custom</b>");

	for (int i = 0; i < 4; i++) {
		num_labels[i].set_width_chars(15);
		num_labels[i].set_margin(5);
		num_labels[i].set_xalign(0.0);
	}

	for (int i = 1; i < 4; i++) {
		rad_buttons[i].set_group(rad_buttons[0]);
	}
	rad_buttons[selection].set_active();

	description_labels[0][0].set_text("Height");
	description_labels[0][1].set_text("Width");
	description_labels[0][2].set_text("Mines");
	description_labels[1][0].set_text("9");
	description_labels[1][1].set_text("9");
	description_labels[1][2].set_text("10");
	description_labels[2][0].set_text("16");
	description_labels[2][1].set_text("16");
	description_labels[2][2].set_text("40");
	description_labels[3][0].set_text("16");
	description_labels[3][1].set_text("30");
	description_labels[3][2].set_text("99");

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			description_labels[i][j].set_width_chars(10);
			description_labels[i][j].set_xalign(0.0);
			top_grid.attach(description_labels[i][j], j + 2, i);
		}
	}

	for (int i = 1; i < 5; i++) {
		top_grid.attach(rad_buttons[i - 1], 0, i);
		top_grid.attach(num_labels[i - 1], 1, i);
	}

	for (int i = 0; i < 3; i++) {
		custom_num_entries[i].set_max_length(3);
		custom_num_entries[i].set_width_chars(3);
		custom_num_entries[i].set_max_width_chars(3);
		custom_num_entries[i].signal_changed().connect(sigc::bind(sigc::mem_fun(*this, &settings_window::on_text_entry_input), i));
		top_grid.attach(custom_num_entries[i], i + 2, 4);
	}
	if (selection == 3) {
		custom_num_entries[0].set_text(std::to_string(game_height));
		custom_num_entries[1].set_text(std::to_string(game_width));
		custom_num_entries[2].set_text(std::to_string(game_mines));
	}

	bottom_grid.attach(apply_button, 0, 0);
	box_separator.set_hexpand(true);
	box_separator.set_margin_bottom(10);
	apply_button.set_expand(false);
	apply_button.signal_clicked().connect(sigc::mem_fun(*this, &settings_window::on_click_apply_button));

	main_grid.set_margin(10);
	main_grid.attach(top_grid, 0, 0);
	main_grid.attach(box_separator, 0, 1);
	main_grid.attach(bottom_grid, 0, 2);

}

void settings_window::on_text_entry_input(int entry_num) {
	auto text = custom_num_entries[entry_num].get_text();
	Glib::ustring new_text;
	bool changed = false;
	for (auto c : text) {
		if (c >= 48 && c <= 57)
			new_text.push_back(c);
		else
			changed = true;
	}
	for (int i = 0; i < 3; i++) {
		rad_buttons[i].set_active(false);
	}
	rad_buttons[3].set_active();

	if (changed) {
		custom_num_entries[entry_num].set_text(new_text);
	}
}

void settings_window::on_click_apply_button() {
	for (int i = 0; i < 4; i++) {
		if (rad_buttons[i].get_active()) {
			selection = i;
			break;
		}
	}

	// Parse the correct settings.
	// For custom games, this does checking against min/max values of the game settings
	switch (selection) {
	case 0:
		game_height = 9; game_width = 9; game_mines = 10; selection = 0; break;
	case 1:
		game_height = 16; game_width = 16; game_mines = 40; selection = 1;  break;
	case 2:
		game_height = 16; game_width = 30; game_mines = 99; selection = 2; break;
	case 3:
		game_height = stoi(std::string(custom_num_entries[0].get_text()));
		game_height = std::max(game_height, 1);
		game_height = std::min(game_height, 99);
		game_width = stoi(std::string(custom_num_entries[1].get_text()));
		game_width = std::max(game_width, 8);
		game_width = std::min(game_width, 99);
		game_mines = stoi(std::string(custom_num_entries[2].get_text()));
		game_mines = std::max(game_mines, 1);
		game_mines = std::min(game_mines, game_height * game_width - 1);
		selection = 3;
		break;
	}
	settings_signal.emit(game_height, game_width, game_mines, selection);
}

settings_window::update_game_signal settings_window::signal_update_game() {
	return settings_signal;
}

game_about_window::game_about_window() {
	set_title("About Game");
	set_child(main_grid);
	set_resizable(false);
	ver_label.set_text("Game Version: " + game_ver);
	ver_label.set_xalign(0.0);
	about_msg.set_margin_top(10);
	about_msg.set_text("J. Wang 2021");
	about_msg.set_xalign(0.0);

	main_grid.set_margin(10);
	main_grid.attach(ver_label, 0, 0);
	main_grid.attach(about_msg, 0, 2);
}

game_code_window::game_code_window() {
	set_title("Game Code");
	set_child(main_grid);
	set_resizable(false);

	game_code_button.set_label("Use Code");

	game_code_text_scroll_window.set_size_request(400, 100);
	game_code_text_scroll_window.set_child(game_code_text);
	game_code_text_scroll_window.set_expand(false);
	game_code_text_scroll_window.set_margin(10);
	game_code_text.set_wrap_mode(Gtk::WrapMode::CHAR);

	edit_mode_button.set_label("Edit mode");
	generate_code_button.set_label("Generate Code");
	copy_code_button.set_label("Copy Code");
	copy_code_button.signal_clicked().connect(sigc::mem_fun(*this, &game_code_window::on_button_copy));
	paste_code_button.set_label("Paste Code");
	paste_code_button.signal_clicked().connect(sigc::mem_fun(*this, &game_code_window::on_button_paste));

	edit_mode_box.append(edit_mode_button);
	edit_mode_box.append(generate_code_button);
	edit_mode_box.append(copy_code_button);
	edit_mode_box.append(paste_code_button);

	main_grid.set_margin(10);
	main_grid.attach(game_code_button, 0, 0);
	main_grid.attach(game_code_text_scroll_window, 0, 1);
	main_grid.attach(edit_mode_box, 0, 2);
}

bool game_code_window::code_button_active() {
	return game_code_button.get_active();
}

std::string game_code_window::get_code() {
	return game_code_text.get_buffer()->get_text();
}

void game_code_window::set_code(std::string new_code) {
	game_code_text.get_buffer()->set_text(new_code);
}

void game_code_window::set_code_button_active(bool active) {
	game_code_button.set_active(active);
}

bool game_code_window::edit_mode_active() {
	return edit_mode_button.get_active();
}

void game_code_window::set_edit_mode_active(bool active) {
	if (edit_mode_button.get_active() != active)
		edit_mode_button.set_active(active);
}

void game_code_window::on_button_copy() {
	get_clipboard()->set_text(game_code_text.get_buffer()->get_text());
}

void game_code_window::on_button_paste() {
	auto content = get_clipboard()->get_content();
	if (!content) {
		get_clipboard()->read_text_async(sigc::mem_fun(*this, &game_code_window::on_clipboard_text_received));
		return;
	}
	auto content_formats = content->ref_formats();
	auto content_types = content_formats->get_gtypes();
	if (content_types[0] == 64) {
		Glib::Value<std::string> text;
		text.init(64);
		content->get_value(text);
		game_code_text.get_buffer()->set_text(text.get());
	}
	else if (content_types[0] == gtk_text_buffer_get_type()) {
		Glib::Value<Glib::RefPtr<Gtk::TextBuffer>> text;
		text.init(content_types[0]);
		content->get_value(text);
		game_code_text.get_buffer()->set_text(text.get()->get_text());
	}
}

void game_code_window::on_clipboard_text_received(Glib::RefPtr<Gio::AsyncResult>& result) {
	Glib::ustring text = get_clipboard()->read_text_finish(result);
	game_code_text.get_buffer()->set_text(text);
}