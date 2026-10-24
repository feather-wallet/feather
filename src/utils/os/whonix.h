// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WHONIX_H
#define FEATHER_WHONIX_H

#include <QString>

// Whonix OS detection.
// When Whonix is detected, Feather assumes Tor is already running and connected:
//   - TorManager will NOT start a managed Tor daemon
//   - TorManager::checkConnection() assumes Tor is connected without verification
//   - Nodes::useSocks5Proxy() returns true (Whonix does not transparently proxy all traffic)
struct WhonixOS {
    // Returns true if the WHONIX environment variable is set (any non-empty value).
    static bool detect();
    static QString version();
};


#endif //FEATHER_WHONIX_H
