/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include <monero_seed/galois_field.hpp>

template<unsigned bits, gf_item primitive>
galois_field<bits, primitive>::galois_field() {
	gf_item b = 1;
	for (gf_item i = 0; i < size_; ++i) {
		log_table[b] = i;
		exp_table[i] = b;
		b <<= 1;
		if ((b & size_) != 0)
			b ^= primitive;
	}
	for (auto i = size_ - 1; i < (2 * size_); ++i) {
		exp_table[i] = exp_table[i - (size_ - 1)];
	}
	for (gf_item i = 2; i < size_; i++) {
		for (gf_item j = 2; j < size_; j++) {
			auto p = mult(i, j);
			if (p == 1) {
				inv_table[i] = j;
				break;
			}
		}
	}
}

template class galois_field<11, 2053>;
