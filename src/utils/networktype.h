// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#pragma once

#include <QObject>

class NetworkType : public QObject
{
    Q_OBJECT

public:
    enum Type : uint8_t {
        MAINNET = 0,
        TESTNET = 1,
        STAGENET = 2
    };
    Q_ENUM(Type)
};
