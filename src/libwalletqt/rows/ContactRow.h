// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_CONTACTROW_H
#define FEATHER_CONTACTROW_H

#include <QString>

struct ContactRow
{
    QString address;
    QString label;

    ContactRow(const QString address_, const QString &label_)
        : address(address_)
        , label(label_) {}
};

#endif //FEATHER_CONTACTROW_H
