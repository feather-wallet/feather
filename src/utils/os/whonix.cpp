// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "whonix.h"

#include "utils/Utils.h"

bool WhonixOS::detect() {
    return !QString::fromLocal8Bit(qgetenv("WHONIX")).isEmpty();
}

QString WhonixOS::version() {
    if (!Utils::fileExists("/etc/whonix_version"))
        return "";

    return Utils::barrayToString(Utils::fileOpen("/etc/whonix_version")).trimmed();
}