// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_ACCOUNTROW_H
#define FEATHER_ACCOUNTROW_H

#include <QString>

struct AccountRow
{
    QString address;
    QString label;
    quint64 balance;
    quint64 unlockedBalance;

    AccountRow(const QString& address_, const QString &label_, uint64_t balance_, uint64_t unlockedBalance_)
            : address(address_)
            , label(label_)
            , balance(balance_)
            , unlockedBalance(unlockedBalance_) {}
};

#endif //FEATHER_ACCOUNTROW_H
