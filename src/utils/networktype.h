// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#pragma once

#include <QObject>
#include <wallet/api/wallet2_api.h>

class NetworkType : public QObject
{
    Q_OBJECT

public:
    enum Type : uint8_t {
        MAINNET = Monero::MAINNET,
        TESTNET = Monero::TESTNET,
        STAGENET = Monero::STAGENET
    };
    Q_ENUM(Type)
};
