// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_INPUT_H
#define FEATHER_INPUT_H

struct Input
{
    QString pubKey;
    quint64 amount;

    explicit Input(uint64_t amount, QString pubkey)
        : pubKey(std::move(pubkey))
        , amount(amount) {}
};

#endif //FEATHER_INPUT_H
