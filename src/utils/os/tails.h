// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TAILSOS_H
#define TAILSOS_H

#include <QString>

// Tails OS detection and integration.
// When Tails is detected, Feather changes Tor-related behavior:
//   - TorManager will NOT start a managed Tor daemon (Tails runs its own)
//   - TorManager::checkConnection() verifies Tor status by checking the systemd target
//     "tails-tor-has-bootstrapped.target" via /bin/systemctl
//   - Nodes::useSocks5Proxy() returns true (Tails does not transparently route all traffic)
//   - Config directory defaults to ~/Persistent/feather_data if persistence is enabled
class TailsOS
{
public:
    // Detects Tails by checking /etc/os-release for TAILS_PRODUCT_NAME or NAME="Tails".
    // Result is cached after first call.
    static bool detect();
    static bool detectDataPersistence();
    static bool detectDotPersistence();
    static QString version();

    static void showDataPersistenceDisabledWarning();
    static void persistXdgMime(const QString& filePath, const QString& data);

    static bool usePersistence;
    static bool rememberChoice;
    static const QString tailsPathData;

    static bool isTails;
    static bool detected;
};

#endif // TAILSOS_H
