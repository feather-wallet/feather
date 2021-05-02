// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_NETWORKMANAGER_H
#define FEATHER_NETWORKMANAGER_H

#include <QNetworkAccessManager>

QNetworkAccessManager* getNetworkTor();
QNetworkAccessManager* getNetworkClearnet();

//void setTorProxy(const QNetworkProxy &proxy);

#endif //FEATHER_NETWORKMANAGER_H
