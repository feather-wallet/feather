// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TRANSFER_H
#define FEATHER_TRANSFER_H

struct Output
{
    QString address;
    quint64 amount;

    explicit Output(uint64_t amount, QString address)
        : address(std::move(address))
        , amount(amount) {}
};

#endif // FEATHER_TRANSFER_H
