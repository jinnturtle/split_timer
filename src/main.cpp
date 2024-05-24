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

#include "Splits.hpp"
#include "version.hpp"

using sys_clock        = std::chrono::high_resolution_clock;
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

void deinit_ncurses();
void draw_splits(
    int x,
    int y,
    int duration_x,
    const Splits* const splits,
    bool is_stopped,
    const milliseconds* const segment_duration);
void init_ncurses();
std::string millis_to_hmsm(uint64_t millis);
// run in interactive mode
int run_ia_mode(const std::string& program_name);
// run in ncurses mode
int run_nc_mode(const std::string& program_name);
void save_split(
    Splits* const splits, const milliseconds* const segment_duration);

/* TODO - separate update and draw cycles, we want to check for input much more
 * often than we want to draw on the screen */

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

std::string millis_to_hmsm(uint64_t millis) {
    unsigned duration_display_hours {
        static_cast<unsigned>(millis / (3600 * 1000))};
    unsigned duration_display_minutes {static_cast<unsigned>(
            (millis % (3600 * 1000)) / (60 * 1000))};
    unsigned duration_display_seconds {static_cast<unsigned>(
            (millis % (60 * 1000)) / 1000)};
    unsigned duration_display_millis {static_cast<unsigned>(
            millis % 1000)};

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

    return duration_buf.str();
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

    enum command_indexes {
        CMD_unknown = 0,
        CMD_list, // print info of all splits
        CMD_next, // select next split (creates new if now at end)
        CMD_quit, // quit the program
        CMD_startstop, // toggle start/stop of current split
        CMD_select, // select a split by name (or position?)
        CMD_rename // change name of current split
    };

    std::cout << prog_name << " " << version_str() << std::endl;

    // TODO - pointing to a callback (not index) would prob be a nicer solution
    std::map<std::string, unsigned> command_dict {
        {"unknown", CMD_unknown},
        {"l", CMD_list},
        {"n", CMD_next},
        {"q", CMD_quit},
        {"s", CMD_startstop},
        {"sel", CMD_select},
        {"ren", CMD_rename}
    };

    std::string def_split_name {"split"};
    Splits splits;
    auto segment_start {sys_clock::now()};
    bool is_stopped {true};

    bool quit {false};
    std::string cmd_ibuf; // command input buffer
    std::map<std::string, unsigned>::iterator cmd_ref {nullptr};
    while (!quit) {
        std::cout << prompt;
        std::cin >> cmd_ibuf;
        cmd_ref = command_dict.find(cmd_ibuf);
        if (cmd_ref == command_dict.end()) {
            cmd_ref = command_dict.find("unknown");
        }

        switch(cmd_ref->second) {
        case CMD_list:
            for (size_t i {0}; i < splits.get_splits_ammount(); ++i) {
                const Split* tmp_sp = splits.get_split(i);
                if (tmp_sp == nullptr) {continue;}

                uint64_t split_millis {tmp_sp->duration};

                if (i == splits.get_active_idx() && !is_stopped) {
                    split_millis += std::chrono::duration_cast<milliseconds>(
                        sys_clock::now() - segment_start).count();
                }

                std::stringstream obuf;
                obuf << i << ":";
                if (i == splits.get_active_idx()) {
                    if (is_stopped) { obuf << "# "; }
                    else { obuf << "> "; }
                } else {
                    obuf << "  ";
                }
                obuf << tmp_sp->name << " : " << millis_to_hmsm(split_millis);
                std::cout << obuf.str() << std::endl;
            }
            break;

        case CMD_next:
            // TODO handle end and not end of list situations differently
            {
                std::stringstream name_buf_ss;
                name_buf_ss << def_split_name
                << "_" << (splits.get_splits_ammount() + 1);
                splits.new_split(name_buf_ss.str());
            }
            break;

        case CMD_quit:
            quit = true;
            break;

        case CMD_startstop:
            if (splits.get_splits_ammount() == 0) {
                std::cout << "no split to start" << std::endl;
                break;
            }

            std::cout << ((is_stopped)? "start" : "stop") << std::endl;
            if (is_stopped) {
                segment_start = sys_clock::now();
            } else {
                milliseconds segment_duration =
                    std::chrono::duration_cast<milliseconds>(
                        sys_clock::now() - segment_start);
                save_split(&splits, &segment_duration);
            }
            is_stopped = !is_stopped;
            break;

        case CMD_select:
            std::cout << "not implemented: " << cmd_ibuf << std::endl;
            break;
        case CMD_rename:
            std::cout << "not implemented: " << cmd_ibuf << std::endl;
            break;
        default:
            std::cout << "unknown command: " << cmd_ibuf << std::endl;
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
                save_split(&splits, &segment_duration);
            } else {
                timer_start = sys_clock::now();
            }

            status_line.msg =
                (is_stopped)? "*** STOPPED ***" : "*** RUNNING ***";
            break;

        case btn_new:
            if (is_stopped == false) {
                save_split(&splits, &segment_duration);
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

void save_split(
    Splits* const splits, const milliseconds* const segment_duration)
{
    splits->add_duration(segment_duration->count());
}
