// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_COINSINFO_H
#define FEATHER_COINSINFO_H

#include <QObject>
#include <QDateTime>
#include <QSet>

class Coins;

class CoinsInfo : public QObject
{
Q_OBJECT

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
    bool change() const;
    QString txNote() const;
    bool keyImagePartial() const;
    bool haveMultisigK() const;
    QStringList multisigInfo() const;

    void setUnlocked(bool unlocked);

private:
    explicit CoinsInfo(QObject *parent);

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
    bool m_change;
    QString m_txNote;
    bool m_keyImagePartial;
    bool m_haveMultisigK;
    QStringList m_multisigInfo;
};

#endif //FEATHER_COINSINFO_H
