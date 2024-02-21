#ifndef SRC_SPLITS_HPP_
#define SRC_SPLITS_HPP_

#include <string>
#include <cstdint>
#include <vector>

struct Split {
	std::string name;
	uint64_t duration;
};

class Splits final {
public:
	Splits();
	~Splits() = default;

	// get a const handle to a split
	// WARNING: refrain from modifying the object at address, fairies may die
	// RETURNS: ptr. to split at index, nullptr on error
	/* TODO - can I tell the compiler to disallow modifications without having
	 * to copy values around?*/
	Split* get_split(size_t index);
	// get number of how many splits there areo
	size_t get_splits_ammount();

	// create and activate a new split
	void new_split(const std::string& name);
	// void rename_split(const std::string& new_name);
	void add_duration(uint64_t duration);
	// void activate_next();
	// void activate_prev();

private:
	std::vector<Split> splits;
	size_t active_split;
};

#endif //define SRC_SPLITS_HPP_
