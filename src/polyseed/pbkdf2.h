// SPDX-License-Identifier: BSD-2-Clause
// SPDX-FileCopyrightText: Copyright 2021 tevador <tevador@gmail.com>

#ifndef PBKDF2_H
#define PBKDF2_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void
crypto_pbkdf2_sha256(const uint8_t* passwd, size_t passwdlen,
                     const uint8_t* salt, size_t saltlen, uint64_t c,
                     uint8_t* buf, size_t dkLen);

#ifdef __cplusplus
}
#endif

#endif