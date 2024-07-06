#ifdef SRC_AI_COMMANDS_HPP_
#error SRC_AI_COMMANDS_HPP_ is defined
#else
#define SRC_AI_COMMANDS_HPP_

#include <chrono>
#include <map>
#include <string>

#include "Splits.hpp"

using sys_clock    = std::chrono::high_resolution_clock;

class Ia_cmd;

// holds references to data needed for ia_mode commands
struct Ia_cmd_data {
    std::map<std::string, Ia_cmd>* command_dict;
    Splits* splits;
    std::string def_split_name;
    sys_clock::time_point segment_start;
    bool is_stopped;
    bool should_quit;
};

class Ia_cmd final {
public:
    Ia_cmd(
        const std::string& doc_name,
        const std::string& doc_desc,
        void* (*callback)(Ia_cmd_data*));

    // execute the function associated with the command
    void* exec(Ia_cmd_data* data);

    const std::string doc_name; // name for documentation purposes
    const std::string doc_desc; // description for documentation purposes

private:
    void* (*callback)(Ia_cmd_data* data); // function assigned to command
};

void* ia_cmd_help(Ia_cmd_data* data);
void* ia_cmd_list(Ia_cmd_data* data);
void* ia_cmd_next(Ia_cmd_data* data);
void* ia_cmd_quit(Ia_cmd_data* data);
void* ia_cmd_startstop(Ia_cmd_data* data);
void* ia_cmd_unimplemented(Ia_cmd_data* data);

#endif //ifdef SRC_AI_COMMANDS_HPP_
