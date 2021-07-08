// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_COINSINFO_H
#define FEATHER_COINSINFO_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <QDateTime>
#include <QSet>

class Coins;

class CoinsInfo : public QObject
{
Q_OBJECT
    Q_PROPERTY(quint64 blockHeight READ blockHeight)
    Q_PROPERTY(QString hash READ hash)
    Q_PROPERTY(quint64 internalOutputIndex READ internalOutputIndex)
    Q_PROPERTY(quint64 globalOutputIndex READ globalOutputIndex)
    Q_PROPERTY(bool spent READ spent)
    Q_PROPERTY(bool frozen READ frozen)
    Q_PROPERTY(quint64 spentHeight READ spentHeight)
    Q_PROPERTY(quint64 amount READ amount)
    Q_PROPERTY(QString displayAmount READ displayAmount)
    Q_PROPERTY(bool rct READ rct)
    Q_PROPERTY(bool keyImageKnown READ keyImageKnown)
    Q_PROPERTY(quint64 pkIndex READ pkIndex)
    Q_PROPERTY(quint32 subaddrIndex READ subaddrIndex)
    Q_PROPERTY(quint32 subaddrAccount READ subaddrAccount)
    Q_PROPERTY(QString address READ address)
    Q_PROPERTY(QString addressLabel READ addressLabel)
    Q_PROPERTY(QString keyImage READ keyImage)
    Q_PROPERTY(quint64 unlockTime READ unlockTime)
    Q_PROPERTY(bool unlocked READ unlocked)
    Q_PROPERTY(QString pubKey READ pubKey)
    Q_PROPERTY(bool coinbase READ coinbase)
    Q_PROPERTY(QString description READ description)

public:
    quint64 blockHeight() const;
    QString hash() const;
    quint64 internalOutputIndex() const;
    quint64 globalOutputIndex() const;
    bool spent() const;
    bool frozen() const;
    quint64 spentHeight() const;
    quint64 amount() const;
    QString displayAmount() const;
    bool rct() const;
    bool keyImageKnown() const;
    quint64 pkIndex() const;
    quint32 subaddrIndex() const;
    quint32 subaddrAccount() const;
    QString address() const;
    QString addressLabel() const;
    QString keyImage() const;
    quint64 unlockTime() const;
    bool unlocked() const;
    QString pubKey() const;
    bool coinbase() const;
    QString description() const;

    void setUnlocked(bool unlocked);

private:
    explicit CoinsInfo(const Monero::CoinsInfo *pimpl, QObject *parent = nullptr);
private:
    friend class Coins;

    quint64 m_blockHeight;
    QString m_hash;
    quint64 m_internalOutputIndex;
    quint64 m_globalOutputIndex;
    bool m_spent;
    bool m_frozen;
    quint64 m_spentHeight;
    quint64 m_amount;
    bool m_rct;
    bool m_keyImageKnown;
    quint64 m_pkIndex;
    quint32 m_subaddrIndex;
    quint32 m_subaddrAccount;
    QString m_address;
    QString m_addressLabel;
    QString m_keyImage;
    quint64 m_unlockTime;
    bool m_unlocked;
    QString m_pubKey;
    bool m_coinbase;
    QString m_description;
};

#endif //FEATHER_COINSINFO_H
