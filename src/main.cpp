#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <sstream>
#include <cstdint>

#include <ncurses.h>

#include "Splits.hpp"

using hi_res_clock = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds  = std::chrono::nanoseconds;

enum {
	btn_down = 'j',
	btn_new = 'n',
	btn_quit = 'q',
	brn_rename = 'n',
	btn_start_stop = 's',
	btn_up = 'k'
};

void init_ncurses();
void deinit_ncurses();
void save_split(
    Splits* const splits, const milliseconds* const segment_duration);

/* TODO - separate update and draw cycles, we want to check for input much more
 * often than we want to draw on the screen */

int main ()
{
	const std::string default_name {"split_1"};
	Splits splits;
	splits.new_split(default_name);

	init_ncurses();

	constexpr unsigned tgt_fps {30};
	constexpr milliseconds tgt_frame_dur {1000/tgt_fps};

	nanoseconds frame_duration {0};
	milliseconds segment_duration {0};
	auto frame_start {hi_res_clock::now()};
	auto timer_start {hi_res_clock::now()};
	std::string status_str {"            "};
	bool is_stopped {true};

	int cmd {0};
	while (cmd != btn_quit) {
		frame_start = hi_res_clock::now();
		segment_duration =
			std::chrono::duration_cast<milliseconds>(frame_start - timer_start);
		cmd = getch();

		switch(cmd) {
		case btn_start_stop:
			is_stopped = !is_stopped;

			if (is_stopped) {
				save_split(&splits, &segment_duration);
			} else {
				timer_start = hi_res_clock::now();
			}

			status_str = (is_stopped)? "*** STOP ***" : "*** RUN  ***";
			break;

		case btn_new:
			if (is_stopped == false) {
				save_split(&splits, &segment_duration);
				timer_start = hi_res_clock::now();
			}

			std::stringstream name_buf_ss;
			name_buf_ss << "split_" << (splits.get_splits_ammount() + 1);
			splits.new_split(name_buf_ss.str());
			break;
		}

		mvprintw(0, 0, status_str.c_str());

		int print_offset_y {1};
		int print_offset_x {0};
		int duration_print_offset_x {40};
		for (size_t i {0}; i < splits.get_splits_ammount(); ++i) {
			move(i + print_offset_y, print_offset_x);
			clrtoeol();
			if (splits.is_active(i)) {
				addstr("> ");
			} else {
				addstr("  ");
			}

			const Split* const current_split {splits.get_split(i)};
			if (current_split == nullptr) { break; } // TODO - report error
			printw("%s", current_split->name.c_str());

			uint64_t duration_display = current_split->duration;
			if (is_stopped == false && splits.is_active(i)) {
				duration_display += segment_duration.count();
			}
			unsigned duration_display_hours {
				static_cast<unsigned>(duration_display / (3600 * 1000))};
			unsigned duration_display_minutes {static_cast<unsigned>(
				(duration_display % (3600 * 1000)) / (60 * 1000))};
			unsigned duration_display_seconds {static_cast<unsigned>(
				(duration_display % (60 * 1000)) / 1000)};
			unsigned duration_display_millis {static_cast<unsigned>(
				duration_display % 1000)};
			std::stringstream duration_buf;
			duration_buf << duration_display_hours << ":";
			if (duration_display_minutes < 10) { duration_buf << "0"; }
			duration_buf << duration_display_minutes << ":";
			if (duration_display_seconds < 10) { duration_buf << "0"; }
			duration_buf << duration_display_seconds << ".";
			for (unsigned j {duration_display_millis +1}; j < 100; j *= 10) {
				duration_buf << "0";
			}
			duration_buf << duration_display_millis;
			mvaddstr(i + print_offset_y, duration_print_offset_x,
				 duration_buf.str().c_str());
			// mvprintw(i + print_offset_y, duration_print_offset_x,
			// 		 "DUR: %u", duration_display);
		}

		frame_duration =
			std::chrono::duration_cast<nanoseconds>(
                hi_res_clock::now() - frame_start);
		std::this_thread::sleep_for(tgt_frame_dur - frame_duration);
	}

	deinit_ncurses();
	return 0;
}

void init_ncurses()
{
	initscr(); //start ncurses mode

	cbreak(); //get raw chan input except specials like Ctrl-C, Ctrl-D
	noecho(); //don't echo chars to terminal
	nodelay(stdscr, true); //don't block on read (e.g. getch())
	keypad(stdscr, true); //enable arrow, function, etc keys
	curs_set(0); //hide the cursor
}

void deinit_ncurses()
{
	endwin();
}

void save_split(
    Splits* const splits, const milliseconds* const segment_duration)
{
	splits->add_duration(segment_duration->count());
}
