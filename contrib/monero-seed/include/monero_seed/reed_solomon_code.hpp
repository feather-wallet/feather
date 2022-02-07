/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include "gf_poly.hpp"

class reed_solomon_code {
public:
	reed_solomon_code(unsigned check_digits);
	void encode(gf_poly& data)  const;
	bool check(const gf_poly& message) const;
private:
	gf_poly get_syndrome(const gf_poly& message) const;
	gf_poly generator;
};
