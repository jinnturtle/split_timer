#include <chrono>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <ncurses.h>

#include "Ia_commands.hpp"
#include "Splits.hpp"
#include "version.hpp"
#include "utils.hpp"

using sys_clock    = std::chrono::high_resolution_clock;
using milliseconds = std::chrono::milliseconds;
using nanoseconds  = std::chrono::nanoseconds;

enum {
    btn_down = 'j',
    btn_new = 'n',
    btn_quit = 'q',
    btn_rename = 'n',
    btn_start_stop = 's',
    btn_up = 'k'
};

void deinit_ncurses();
void draw_splits(
    int x,
    int y,
    int duration_x,
    const Splits* const splits,
    bool is_stopped,
    const milliseconds* const segment_duration);
void init_ncurses();
// run in interactive mode
int run_ia_mode(const std::string& program_name);
// run in ncurses mode
int run_nc_mode(const std::string& program_name);

/* TODO - (nc mode) separate update and draw cycles, we want to check for input
 * much more often than we want to draw on the screen */

int main (int argc, char** argv)
{
    const std::string program_name {"split_timer"};
    int retcode {0};

    if (argc > 1 && !strcmp(argv[1], "-n")) {
        retcode = run_nc_mode(program_name);
    }
    else { retcode = run_ia_mode(program_name); }

    return retcode;
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

void draw_splits(
    int x,
    int y,
    int duration_x,
    const Splits* const splits,
    bool is_stopped,
    const milliseconds* const segment_duration)
{
    for (size_t i {0}; i < splits->get_splits_ammount(); ++i) {
        move(i + y, x);
        clrtoeol();
        if (splits->is_active(i)) {
            addstr("> ");
        } else {
            addstr("  ");
        }

        const Split* const current_split {splits->get_split(i)};
        if (current_split == nullptr) { break; } // TODO - report error
        printw("%s", current_split->name.c_str());

        uint64_t duration_display = current_split->duration;
        if (is_stopped == false && splits->is_active(i)) {
            duration_display += segment_duration->count();
        }

        mvaddstr(i + y, duration_x, millis_to_hmsm(duration_display).c_str());
    }
}

int run_ia_mode(const std::string& prog_name)
{
    std::string prompt("> ");

    std::cout << prog_name << " " << version_str() << std::endl;

    std::map<std::string, Ia_cmd> command_dict {
        {"l", {"list", "print the splits", &ia_cmd_list}},
        {"n", {"next", "create/select next split", &ia_cmd_next}},
        {"q", {"quit", "terminate the program", &ia_cmd_quit}},
        {"s", {"start/stop", "start/stop split", &ia_cmd_startstop}},
        {"h", {"help", "display help", &ia_cmd_help}},
        {"ren", {"rename", "(not implemented)", &ia_cmd_unimplemented}},
        {"sel", {"select", "(not implemented)", &ia_cmd_unimplemented}}
    };

    Splits splits;
    Ia_cmd_data cmd_data {
        .command_dict = &command_dict,
        .splits = &splits,
        .def_split_name = "split",
        .segment_start = sys_clock::now(),
        .is_stopped = true,
        .should_quit = false};

    std::string cmd_ibuf; // command input buffer
    std::map<std::string, Ia_cmd>::iterator cmd_ref {nullptr};
    while (!cmd_data.should_quit) {
        std::cout << prompt;
        std::cin >> cmd_ibuf;
        cmd_ref = command_dict.find(cmd_ibuf);
        if (cmd_ref != command_dict.end()) {
            cmd_ref->second.exec(&cmd_data);
        } else {
            std::cout << "unknown command '" << cmd_ibuf << "'" << std::endl;
        }
    }

    return 0;
}

// TODO ugle, meant as POC at most, review and rewrite plz
int run_nc_mode(const std::string& prog_name)
{
    init_ncurses();

    const std::string def_split_name {"split"};
    Splits splits;
    splits.new_split(def_split_name);

    constexpr unsigned tgt_fps {30};
    constexpr milliseconds tgt_frame_dur {1000/tgt_fps};

    nanoseconds frame_duration {0};
    milliseconds segment_duration {0};
    auto frame_start {sys_clock::now()};
    auto timer_start {sys_clock::now()};
    bool is_stopped {true};

    struct Status_line {
        int x, y;
        std::string msg;
    } status_line {0, 0, {prog_name + " " + version_str()}};

        int cmd {0};
    while (cmd != btn_quit) {
        frame_start = sys_clock::now();
        segment_duration =
            std::chrono::duration_cast<milliseconds>(frame_start - timer_start);
        cmd = getch();

        switch(cmd) {
        case btn_start_stop:
            is_stopped = !is_stopped;

            if (is_stopped) {
                splits.add_duration(segment_duration.count());
            } else {
                timer_start = sys_clock::now();
            }

            status_line.msg =
                (is_stopped)? "*** STOPPED ***" : "*** RUNNING ***";
            break;

        case btn_new:
            if (is_stopped == false) {
                splits.add_duration(segment_duration.count());
                timer_start = sys_clock::now();
            }

            {
                std::stringstream name_buf_ss;
                name_buf_ss << def_split_name
                << "_" << (splits.get_splits_ammount() + 1);
                splits.new_split(name_buf_ss.str());
            }
            break;
        }

        mvprintw(0, 0, status_line.msg.c_str());
        clrtoeol();
        draw_splits(0, 1, 40, &splits, is_stopped, &segment_duration);

        frame_duration =
            std::chrono::duration_cast<nanoseconds>(
                sys_clock::now() - frame_start);
        std::this_thread::sleep_for(tgt_frame_dur - frame_duration);
    }

    deinit_ncurses();
    return 0;
}
