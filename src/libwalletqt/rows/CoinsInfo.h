// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_COINSINFO_H
#define FEATHER_COINSINFO_H

#include <QString>

struct CoinsInfo
{
    quint64 blockHeight;
    QString hash;
    quint64 internalOutputIndex;
    quint64 globalOutputIndex;
    bool spent;
    bool frozen;
    quint64 spentHeight;
    quint64 amount;
    bool rct;
    bool keyImageKnown;
    quint64 pkIndex;
    quint32 subaddrIndex;
    quint32 subaddrAccount;
    QString address;
    QString addressLabel;
    QString keyImage;
    quint64 unlockTime;
    bool unlocked;
    QString pubKey;
    bool coinbase;
    QString description;
    bool change;
    QString txNote;

    QString getAddressLabel() const;
    QString displayAmount() const;
    void setUnlocked(bool unlocked);

    explicit CoinsInfo();
};

#endif //FEATHER_COINSINFO_H
