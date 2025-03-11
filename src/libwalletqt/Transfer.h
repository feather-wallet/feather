// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TRANSFER_H
#define TRANSFER_H

struct Transfer
{
    QString address;
    quint64 amount;

    explicit Transfer(uint64_t amount_, QString address_)
        : address(std::move(address_))
        , amount(amount_) {}
};

#endif // TRANSFER_H
