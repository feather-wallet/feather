// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_SEED_H
#define FEATHER_SEED_H

#include "constants.h"
#include "libwalletqt/Wallet.h"
#include "monero_seed/monero_seed.hpp"
#include "polyseed/polyseed.h"
#include "utils/AppData.h"

#include <QString>
#include <QStringList>

#include <sstream>
#include <utility>

struct Seed {
    enum Type {
        MONERO = 0, // 25 word seeds
        TEVADOR,    // 14 word seeds
        POLYSEED,   // 16 word seeds
    };

    NetworkType::Type networkType = NetworkType::MAINNET;

    QString coin;
    QString language;
    Type type = Type::TEVADOR;

    QStringList mnemonic;
    QString spendKey;
    QString correction;

    time_t time{};
    int restoreHeight = 0;

    QString errorString;

    explicit Seed();
    explicit Seed(Type type, NetworkType::Type networkType = NetworkType::MAINNET, QString language = "English");
    explicit Seed(Type type, QStringList mnemonic, NetworkType::Type networkType = NetworkType::MAINNET);
    void setRestoreHeight(int height);

private:
    void setRestoreHeight();

    QStringList m_insecureSeeds = {
        "0000000000000000000000000000000000000000000000000000000000000000",
        "1111111111111111111111111111111111111111111111111111111111111111",
        "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"
    };

    QMap<Seed::Type, int> m_seedLength {
        {Type::MONERO, 25},
        {Type::TEVADOR, 14},
        {Type::POLYSEED, 16}
    };
};


#endif //FEATHER_SEED_H
