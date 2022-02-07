/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once

#include <string>
#include <assert.h>

class wordlist {
public:
	static constexpr size_t size = 2048;
	static const wordlist english;
	const std::string& get_word(unsigned i) const {
		assert(i < size);
		return values_[i];
	}
	int parse(const std::string& word) const;
private:
	wordlist(const std::string(&values)[size]) : values_(values)
	{
	}
	const std::string(&values_)[size];
};
