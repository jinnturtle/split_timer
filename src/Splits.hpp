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
	const Split* get_split(size_t index) const;
	// get number of how many splits there areo
	size_t get_splits_ammount() const;
	// check if split at index is active
	bool is_active(size_t index) const;

	void add_duration(uint64_t duration);
	// void activate_next();
	// void activate_prev();
	// create and activate a new split
	void new_split(const std::string& name);
	// void rename_split(const std::string& new_name);

private:
	std::vector<Split> splits;
	size_t active_split;
};

#endif //define SRC_SPLITS_HPP_
