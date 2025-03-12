// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SUBADDRESSROW_H
#define FEATHER_SUBADDRESSROW_H

#include <QString>

struct SubaddressRow
{
    QString address;
    QString label;
    bool used = false;
    bool hidden = false;
    bool pinned = false;

    SubaddressRow(const QString& address, const QString &label, bool used, bool hidden, bool pinned)
        : address(address)
        , label(label)
        , used(used)
        , hidden(hidden)
        , pinned(pinned) {}
};

#endif //FEATHER_SUBADDRESSROW_H
