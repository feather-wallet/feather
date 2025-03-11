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

    SubaddressRow(const QString& address_, const QString &label_, bool used_, bool hidden_, bool pinned_)
        : address(address_)
        , label(label_)
        , used(used_)
        , hidden(hidden_)
        , pinned(pinned_) {}
};

#endif //FEATHER_SUBADDRESSROW_H
