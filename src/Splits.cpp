#include "Splits.hpp"

#include <limits>

Splits::Splits()
: active_split {std::numeric_limits<size_t>::max()}
{}

size_t Splits::get_active_idx() const { return this->active_split; }

const Split* Splits::get_split(size_t index) const
{
	if (index < this->splits.size()) {
		return &this->splits[index];
	} else {
		return nullptr;
	}
}

size_t Splits::get_splits_ammount() const { return this->splits.size(); }

bool Splits::is_active(size_t index) const
{
	return index == this->active_split;
}

void Splits::add_duration(uint64_t duration)
{
	this->splits[this->active_split].duration += duration;
}

void Splits::new_split(const std::string& name)
{
	this->splits.push_back({name, 0});
	this->active_split = this->splits.size()-1;
}
