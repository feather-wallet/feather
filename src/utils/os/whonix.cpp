// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "whonix.h"

#include "utils/Utils.h"

// Detection relies on the WHONIX environment variable, which is set by Whonix's
// /etc/profile.d scripts. Any non-empty value triggers detection.
bool WhonixOS::detect() {
    return !QString::fromLocal8Bit(qgetenv("WHONIX")).isEmpty();
}

QString WhonixOS::version() {
    if (!Utils::fileExists("/etc/whonix_version"))
        return "";

    return Utils::barrayToString(Utils::fileOpen("/etc/whonix_version")).trimmed();
}