/*
    Copyright (c) 2020 tevador <tevador@gmail.com>
    All rights reserved.
*/

#pragma once
#include "galois_field.hpp"

class gf_elem {
public:
    static constexpr gf_item size() {
        return gf_2048::size();
    }
    constexpr gf_elem() : value_(0)
    {
    }
    constexpr gf_elem(gf_item value) : value_(value)
    {
    }
    gf_elem& operator+=(gf_elem x) {
        value_ ^= x.value_;
        return *this;
    }
    gf_elem& operator|=(gf_elem x) {
        value_ |= x.value_;
        return *this;
    }
    gf_elem& operator-=(gf_elem x) {
        value_ ^= x.value_;
        return *this;
    }
    gf_elem& operator*=(gf_elem x) {
        value_ = field.mult(value_, x.value_);
        return *this;
    }
    friend gf_elem operator+(gf_elem left, gf_elem right) {
        left += right;
        return left;
    }
    friend gf_elem operator-(gf_elem left, gf_elem right) {
        left -= right;
        return left;
    }
    friend gf_elem operator*(gf_elem left, gf_elem right) {
        left *= right;
        return left;
    }
    friend bool operator==(gf_elem lhs, gf_elem rhs) {
        return lhs.value_ == rhs.value_;
    }
    friend bool operator!=(gf_elem lhs, gf_elem rhs) {
        return !(lhs == rhs);
    }
    gf_elem& inverse() {
        value_ = field.inverse(value_);
        return *this;
    }
    gf_elem& exp() {
        value_ = field.exp(value_);
        return *this;
    }
    gf_item value() const {
        return value_;
    }
private:
	static const gf_2048 field;
	gf_item value_;
};
