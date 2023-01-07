/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include <cstddef>
#include <cstdint>

class secure_random {
public:
	static void gen_bytes(void* output, size_t size);
};

