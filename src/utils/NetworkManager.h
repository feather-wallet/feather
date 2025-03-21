// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_NETWORKMANAGER_H
#define FEATHER_NETWORKMANAGER_H

#include <QNetworkAccessManager>

QNetworkAccessManager* getNetworkSocks5();
QNetworkAccessManager* getNetworkClearnet();

QNetworkAccessManager* getNetwork(const QString &address = "");

#endif //FEATHER_NETWORKMANAGER_H
