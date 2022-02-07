/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include "gf_elem.hpp"
#include <cassert>
#include <algorithm>
#include <iostream>

class gf_poly {
public:
	static constexpr size_t max_degree = 13;
	gf_poly() : degree_(0) //zero polynomial
	{
	}
	gf_poly(gf_elem coeff, unsigned degree); //monomial
	gf_poly(unsigned degree) : gf_poly(1, degree)
	{
	}
	gf_poly(gf_elem coeff[], unsigned degree);
	unsigned degree() const {
		return degree_;
	}
	void set_degree();
	void set_degree(unsigned degree) {
		degree_ = degree;
	}
	bool is_zero() const {
		return degree_ == 0 && coeff_[0] == 0;
	}
	gf_elem operator[](unsigned i) const {
		return coeff_[i];
	}
	gf_elem& operator[](unsigned i) {
		return coeff_[i];
	}
	gf_elem operator()(gf_elem x) const; //evaluate at point x
	gf_poly& operator+=(const gf_poly& x);
	gf_poly& operator-=(const gf_poly& x);
	gf_poly& operator*=(gf_elem x);
	gf_poly& operator*=(const gf_poly& x);
	friend gf_poly operator*(const gf_poly& lhs, const gf_poly& rhs) {
		gf_poly result(lhs);
		return result *= rhs;
	}
	friend gf_poly operator+(const gf_poly& lhs, const gf_poly& rhs) {
		gf_poly result(lhs);
		return result += rhs;
	}
	static gf_poly div_rem(const gf_poly& nom, const gf_poly& x, gf_poly& rem);
	friend std::ostream& operator<<(std::ostream& os, const gf_poly& poly);
private:
	gf_elem coeff_[2 * (max_degree + 1)] = { };
	unsigned degree_;
};
