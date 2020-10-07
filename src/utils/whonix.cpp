// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "whonix.h"

#include "utils/utils.h"

bool WhonixOS::detect() {
    return !QString::fromLocal8Bit(qgetenv("WHONIX")).isEmpty();
}