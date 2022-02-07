/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void pbkdf2_hmac_sha256(const uint8_t* password, size_t pw_size,
	const uint8_t* salt, size_t salt_size,
	int iterations, uint8_t* key, size_t key_size);

#ifdef __cplusplus
}
#endif
