// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "Prestium.h"

#include <QString>
#include <QSysInfo>

#include "utils/Utils.h"

#define PRESTIUM_DEFAULT_PORT 4448

bool Prestium::detect()
{
    return QSysInfo::prettyProductName().contains("Prestium");
}

int Prestium::i2pPort()
{
    QString portStr = qgetenv("PRESTIUM_FEATHER_I2P_PORT");
    if (portStr.isEmpty()) {
        return PRESTIUM_DEFAULT_PORT;
    }

    int port = portStr.toInt();
    if (port < 1024 || port > 65535) {
        return PRESTIUM_DEFAULT_PORT;
    }

    return port;
}