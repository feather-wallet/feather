/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include <monero_seed/reed_solomon_code.hpp>
#include <cassert>

reed_solomon_code::reed_solomon_code(unsigned check_digits) : generator(1, 0) {
	for (unsigned i = 0; i < check_digits; ++i) {
		gf_poly binom = gf_poly(1, 1);
		binom[0] = gf_elem(i + 1).exp();
		generator *= binom;
	}
	assert(generator.degree() == check_digits);
}

void reed_solomon_code::encode(gf_poly& data) const {
	data *= gf_poly(generator.degree());
	gf_poly rem;
	gf_poly::div_rem(data, generator, rem);
	data -= rem;
}

bool reed_solomon_code::check(const gf_poly& message) const {
	auto syndrome = get_syndrome(message);
	return syndrome.is_zero();
}

gf_poly reed_solomon_code::get_syndrome(const gf_poly& message) const {
	gf_poly syndrome;
	for (unsigned i = 1; i <= generator.degree(); ++i) {
		syndrome[i - 1] = message(gf_elem(i).exp());
	}
	return syndrome;
}
