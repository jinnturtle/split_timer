#include "Ia_commands.hpp"

#include <iostream>
#include <sstream>

#include "utils.hpp"

using milliseconds = std::chrono::milliseconds;

Ia_cmd::Ia_cmd(
    const std::string& doc_name,
    const std::string& doc_desc,
    void* (*callback)(Ia_cmd_data*))
: doc_name{doc_name}
, doc_desc{doc_desc}
, callback{callback}
{}

void* Ia_cmd::exec(Ia_cmd_data* data)
{
    return this->callback(data);
}

// defines for callbacks ----------------------------------------

void* ia_cmd_help(Ia_cmd_data* data)
{
    std::cout
    << "\n*** HELP ***\n\n"
    << "COMMANDS" << std::endl;

    for (auto& el : *data->command_dict) {
        std::cout
        << "\t" << el.first
        << "\t(" << el.second.doc_name
        << ")\t" << el.second.doc_desc << std::endl;
    }

    return nullptr;
}

void* ia_cmd_list(Ia_cmd_data* data)
{
    Splits* splits {data->splits};

    for (size_t i {0}; i < splits->get_splits_ammount(); ++i) {
        const Split* tmp_sp = splits->get_split(i);
        if (tmp_sp == nullptr) {continue;}

        uint64_t split_millis {tmp_sp->duration};

        if (i == splits->get_active_idx() && !data->is_stopped) {
            split_millis += std::chrono::duration_cast<milliseconds>(
                sys_clock::now() - data->segment_start).count();
        }

        std::stringstream obuf;
        obuf << i << ":";
        if (i == splits->get_active_idx()) {
            if (data->is_stopped) { obuf << "# "; }
            else { obuf << "> "; }
        } else {
            obuf << "  ";
        }
        obuf << tmp_sp->name << " : " << millis_to_hmsm(split_millis);
        std::cout << obuf.str() << std::endl;
    }

    return nullptr;
}

void* ia_cmd_next(Ia_cmd_data* data)
{
    // TODO handle end and not end of list situations differently
    {
        std::stringstream name_buf_ss;
        name_buf_ss << data->def_split_name
                    << "_" << (data->splits->get_splits_ammount() + 1);
        data->splits->new_split(name_buf_ss.str());
    }

    return nullptr;
}

void* ia_cmd_quit(Ia_cmd_data* data)
{
    data->should_quit = true;
    return nullptr;
}

void* ia_cmd_startstop(Ia_cmd_data* data)
{
    if (data->splits->get_splits_ammount() == 0) {
        std::cout << "no split to start" << std::endl;
        return nullptr;
    }

    std::cout << ((data->is_stopped)? "start" : "stop") << std::endl;
    if (data->is_stopped) {
        data->segment_start = sys_clock::now();
    } else {
        milliseconds segment_duration =
            std::chrono::duration_cast<milliseconds>(
                sys_clock::now() - data->segment_start);
        data->splits->add_duration(segment_duration.count());
    }
    data->is_stopped = !data->is_stopped;

    return nullptr;
}

void* ia_cmd_unimplemented(Ia_cmd_data* data)
{
    (void)(data); // unused - hush warning

    std::cout << "This function is not implemented yet." << std::endl;
    return nullptr;
}
