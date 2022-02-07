/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include <cstdint>
#include <cassert>

typedef uint_least16_t gf_storage;
typedef uint_fast16_t gf_item;

template<unsigned bits, gf_item primitive>
class galois_field {
public:
	galois_field();
	gf_item inverse(gf_item i) const {
		assert(i != 0);
		return i > 1 ? inv_table[i] : 1;
	}
	gf_item mult(gf_item a, gf_item b) const {
		if (b == 0 || a == 0)
			return 0;
		if (b == 1)
			return a;
		if (a == 1)
			return b;
		return exp_table[log_table[a] + log_table[b]];
	}
	gf_item exp(gf_item i) const {
		return exp_table[i];
	}
	static constexpr unsigned size() {
		return bits;
	}
	static constexpr unsigned elements() {
		return size_;
	}
private:
	static_assert(bits <= 14, "field is too large");
	static constexpr gf_item size_ = 1u << bits;
	gf_storage log_table[size_];
	gf_storage exp_table[2 * size_];
	gf_storage inv_table[size_];
};

using gf_2048 = galois_field<11, 2053>;
