/*
	Copyright (c) 2020 tevador <tevador@gmail.com>
	All rights reserved.
*/

#include "sha256/hash_impl.h"

#include <string.h>
#include <stdint.h>

#define BLOCK_SIZE 32

typedef struct pbkdf_state {
	int iterations;
	uint32_t block_count;
	uint8_t block[BLOCK_SIZE];
} pbkdf_state;

static void pbkdf2_transform(const uint8_t* password, size_t pw_size,
	const uint8_t* salt, size_t salt_size, pbkdf_state* state)
{
	hmac_sha256_state hash_state;
	hmac_sha256_initialize(&hash_state, password, pw_size);
	hmac_sha256_write(&hash_state, salt, salt_size);
	uint8_t block_buff[4];
	//big endian
	block_buff[0] = state->block_count >> 24;
	block_buff[1] = state->block_count >> 16;
	block_buff[2] = state->block_count >> 8;
	block_buff[3] = state->block_count;
	hmac_sha256_write(&hash_state, block_buff, sizeof(block_buff));
	hmac_sha256_finalize(&hash_state, state->block);
	hmac_sha256_initialize(&hash_state, password, pw_size);

	uint8_t temp[BLOCK_SIZE];
	memcpy(temp, state->block, BLOCK_SIZE);

	for (unsigned i = 2; i <= state->iterations; ++i) {
		hmac_sha256_write(&hash_state, temp, sizeof(temp));
		hmac_sha256_finalize(&hash_state, temp);
		for (unsigned j = 0; j < BLOCK_SIZE; ++j) {
			state->block[j] ^= temp[j];
		}
		hmac_sha256_initialize(&hash_state, password, pw_size);
	}

	state->block_count++;
}

void pbkdf2_hmac_sha256(const uint8_t* password, size_t pw_size,
	uint8_t* salt, size_t salt_size,
	int iterations, uint8_t* key, size_t key_size)
{
	pbkdf_state state = {
		.block_count = 1,
		.iterations = iterations
	};
	while (key_size > 0) {
		pbkdf2_transform(password, pw_size, salt, salt_size, &state);
		size_t block_size = key_size > BLOCK_SIZE ? BLOCK_SIZE : key_size;
		memcpy(key, state.block, block_size);
		key += BLOCK_SIZE;
		key_size -= BLOCK_SIZE;
	}
}
