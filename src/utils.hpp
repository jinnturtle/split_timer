#ifdef SRC_UTILS_HPP_
#error SRC_UTILS_HPP_ is defined
#else
#define SRC_UTILS_HPP_

#include <cstdint>
#include <sstream>
#include <string>

// return hmsm textual time representation from milliseconds
std::string millis_to_hmsm(uint64_t millis);

#endif //ifdef SRC_UTILS_HPP_
