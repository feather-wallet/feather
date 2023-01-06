/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include <monero_seed/gf_poly.hpp>

gf_poly::gf_poly(gf_elem coeff, unsigned degree) : degree_(0) {
	coeff_[degree] = coeff;
	if (coeff != 0) {
		degree_ = degree;
	}
}

gf_poly::gf_poly(gf_elem coeff[], unsigned degree) : degree_(0) {
	assert(degree <= max_degree);
	for (unsigned i = 0; i <= degree; ++i) {
		coeff_[i] = coeff[i];
		if (coeff_[i] != 0) {
			degree_ = i;
		}
	}
}

void gf_poly::set_degree() {
	degree_ = 0;
	for (unsigned i = 0; i <= max_degree; ++i) {
		if (coeff_[i] != 0) {
			degree_ = i;
		}
	}
}

gf_poly& gf_poly::operator+=(const gf_poly& x) {
	auto degree = std::max(degree_, x.degree_);
	degree_ = 0;
	for (unsigned i = 0; i <= degree; ++i) {
		coeff_[i] += x[i];
		if (coeff_[i] != 0) {
			degree_ = i;
		}
	}
	return *this;
}

gf_poly& gf_poly::operator-=(const gf_poly& x) {
	auto degree = std::max(degree_, x.degree_);
	degree_ = 0;
	for (unsigned i = 0; i <= degree; ++i) {
		coeff_[i] -= x[i];
		if (coeff_[i] != 0) {
			degree_ = i;
		}
	}
	return *this;
}

gf_poly& gf_poly::operator*=(gf_elem x) {
	auto degree = degree_;
	for (unsigned i = 0; i <= degree; ++i) {
		coeff_[i] *= x;
		if (coeff_[i] != 0) {
			degree_ = i;
		}
	}
	return *this;
}

gf_poly& gf_poly::operator*=(const gf_poly& x) {
	gf_poly result;
	for (unsigned i = 0; i <= degree_; ++i) {
		for (unsigned j = 0; j <= x.degree_; ++j) {
			result.coeff_[i + j] += coeff_[i] * x[j];
		}
	}
	for (unsigned i = 0; i <= degree_ + x.degree_; ++i) {
		if (result.coeff_[i] != 0) {
			result.degree_ = i;
		}
	}
	return *this = result;
}

gf_elem gf_poly::operator()(gf_elem x) const {
	if (x == 0) {
		return coeff_[0];
	}
	//Horner's method
	auto result = coeff_[degree_];
	for (unsigned i = degree_ - 1; i < degree_; --i) {
		result = result * x + coeff_[i];
	}
	return result;
}

gf_poly gf_poly::div_rem(const gf_poly& nom, const gf_poly& x, gf_poly& rem) {
	assert(!x.is_zero());
	gf_poly quotient;
	rem = gf_poly(nom);
	gf_elem divisor_term = x[x.degree_];
	divisor_term.inverse();
	while (rem.degree_ >= x.degree_ && !rem.is_zero()) {
		auto degree_diff = rem.degree_ - x.degree_;
		auto digit = rem[rem.degree_] * divisor_term;
		gf_poly mono(digit, degree_diff);
		gf_poly term = x * mono;
		quotient += mono;
		rem -= term;
	}

	return quotient;
}



std::ostream& operator<<(std::ostream& os, const gf_poly& poly) {
	bool term = false;
	for (unsigned i = poly.degree_; i <= poly.degree_; --i) {
		if (poly[i] != 0 || i == 0) {
			if (term)
				std::cout << " + ";
			if (i == 0 || poly[i] != 1) {
				std::cout << poly[i].value();
			}
			if (i != 0) {
				if (i == 1)
					std::cout << "x";
				else
					std::cout << "x**" << i;
			}
			term = true;
		}
	}
	return os;
}
