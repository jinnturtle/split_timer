#include "utils.hpp"

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
