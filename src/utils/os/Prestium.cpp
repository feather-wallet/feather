// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Prestium.h"

#include <QString>
#include <QSysInfo>

#include "utils/Utils.h"

bool Prestium::detect()
{
    return QSysInfo::prettyProductName().contains("Prestium");
}