#include "game_window.h"
#include <iostream>

game_window::game_window() {
	//Base initialization
	set_title("Minesweeper");
	set_default_size(1000, 636);
	initialize_icons();
	g_fixed.set_size_request(1000, 636);
	set_child(g_fixed);

	//Main Drawingarea
	main_da.set_content_width(1000);
	main_da.set_content_height(636);
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

	//--gestureclicks for mines da
	lm_click = Gtk::GestureClick::create();
	lm_click->set_button(1);
	mines_da.add_controller(lm_click);
	rm_click = Gtk::GestureClick::create();
	rm_click->set_button(3);
	mines_da.add_controller(rm_click);

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

void game_window::initialize_icons() {
	for (int i = 0; i < 10; i++) {
		g_icons[(std::string)("c_" + std::to_string(i))] = Gdk::Pixbuf::create_from_resource("/counter/c_" + std::to_string(i) + ".png");
	}

	g_icons["ok_head"] = Gdk::Pixbuf::create_from_resource("/headicons/ok_head.png");
	g_icons["lost_head"] = Gdk::Pixbuf::create_from_resource("/headicons/lost_head.png");
	g_icons["clicked_head"] = Gdk::Pixbuf::create_from_resource("/headicons/clicked_head.png");
	g_icons["won_head"] = Gdk::Pixbuf::create_from_resource("/headicons/won_head.png");
	g_icons["reset_head"] = Gdk::Pixbuf::create_from_resource("/headicons/reset_head.png");

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

	if (!main_da_surface) {	// If main_da_surface==NULL, initialize
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

	cr->set_source(main_da_surface, 0, 0);
	cr->paint();
}

void game_window::on_mines_da_draw(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height) {
	if (!mines_da_surface) {
		mines_da_surface = Cairo::ImageSurface::create(Cairo::Surface::Format::ARGB32, width, height);
		auto new_cr = Cairo::Context::create(mines_da_surface);

		for (int i = 0; i < m_game.get_rows(); i++) {
			for (int j = 0; j < m_game.get_cols(); j++) {
				Gdk::Cairo::set_source_pixbuf(new_cr, g_icons["unchecked"], 32 * j, 32 * i);
				new_cr->paint();
			}
		}
	}

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

			m_game.reset();
			timer_connection.disconnect();
			sec_count = 0;
			main_da_surface = nullptr;
			mines_da_surface = nullptr;

			main_da.queue_draw();
			mines_da.queue_draw();
		}
	}
}

void game_window::on_mines_da_click_begin(Gdk::EventSequence* es, int button_num) {

	if (m_game.get_game_state() == minesweeper::g_states::lost || m_game.get_game_state() == minesweeper::g_states::won)
		return;

	if (button_num == 1) {
		double pix_x, pix_y;
		lm_click->get_point(es, pix_x, pix_y);
		last_pos.first = pix_x;
		last_pos.second = pix_y;
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
	else if (3) {
		if (!rclick_released)
			return;

		double pix_x, pix_y;
		rm_click->get_point(es, pix_x, pix_y);
		last_pos.first = pix_x;
		last_pos.second = pix_y;
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
			redraw_cells(cells, game_window::draw_selection::flag);
			m_game.set_flag(grid_x, grid_y);
		}
	}
}

void game_window::on_mines_da_click_update(Gdk::EventSequence* es, int button_num) {

	if (m_game.get_game_state() == minesweeper::g_states::lost || m_game.get_game_state() == minesweeper::g_states::won)
		return;

	double pix_x, pix_y;
	if (button_num == 1)
		lm_click->get_point(es, pix_x, pix_y);
	else if (button_num == 3)
		rm_click->get_point(es, pix_x, pix_y);
	last_pos.first = pix_x;
	last_pos.second = pix_y;
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

	if (m_game.get_game_state() == minesweeper::g_states::lost || m_game.get_game_state() == minesweeper::g_states::won)
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
			if (m_game.get_game_state() == minesweeper::g_states::new_game)
				timer_connection = Glib::signal_timeout().connect_seconds(sigc::mem_fun(*this, &game_window::timer_handler), 1);
			auto cells = m_game.l_click_clear(grid_x, grid_y);
			redraw_cells(cells, game_window::draw_selection::reveal);
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
				if (tile_type >= 0)
					continue;

				if (tile_type == -2) {
					Gdk::Cairo::set_source_pixbuf(cr, g_icons["exploded"], 32 * x, 32 * y);
				}
				else {
					if (m_game.get_tile_state(x, y) == minesweeper::states::flagged)
						Gdk::Cairo::set_source_pixbuf(cr, g_icons["flagged"], 32 * x, 32 * y);
					else
						Gdk::Cairo::set_source_pixbuf(cr, g_icons["mine"], 32 * x, 32 * y);
				}
				cr->paint();
			}
		}
	}
	if (m_game.get_game_state() == minesweeper::g_states::won) {
		timer_connection.disconnect();//do win stuff here;
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
}

void game_window::redraw_cells(std::vector<std::pair<int, int>>& cells, game_window::draw_selection draw_type) {

	if (cells.empty())
		return;

	auto cr = Cairo::Context::create(mines_da_surface);

	if (draw_type == game_window::draw_selection::flag) {
		for (auto& cell : cells) {
			if (m_game.get_tile_state(cell.first, cell.second) == minesweeper::states::covered) {
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["flagged"], cell.first * 32, cell.second * 32);
				cr->paint();
				m_game.dec_remaining();
			}
			else if (m_game.get_tile_state(cell.first, cell.second) == minesweeper::states::flagged) {
				Gdk::Cairo::set_source_pixbuf(cr, g_icons["unchecked"], cell.first * 32, cell.second * 32);
				cr->paint();
				m_game.inc_remaining();
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
	for (int i = 2; i >= 0; i--) {
		digits[i] = mines % 10;
		mines /= 10;
	}

	for (int i = 0; i < 3; i++) {
		Gdk::Cairo::set_source_pixbuf(cr, g_icons["c_" + std::to_string(digits[i])], x_offset + i * icon_width, y_offset);
		cr->paint();
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
