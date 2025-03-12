// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_CONTACTROW_H
#define FEATHER_CONTACTROW_H

#include <QString>

struct ContactRow
{
    QString address;
    QString label;

    ContactRow(const QString address, const QString& label)
        : address(address)
        , label(label) {}
};

#endif //FEATHER_CONTACTROW_H
