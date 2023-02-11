// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Prestium.h"

#include <QString>
#include <QSysInfo>

#include "utils/Utils.h"

bool Prestium::detect()
{
    // Temporary detect
    if (Utils::fileExists("/etc/hostname")) {
        QByteArray data = Utils::fileOpen("/etc/hostname");
        if (data == "prestium\n") {
            return true;
        }
    }

    return QSysInfo::prettyProductName().contains("Prestium");
}