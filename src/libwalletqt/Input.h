// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_INPUT_H
#define FEATHER_INPUT_H

#include <QObject>

class Input : public QObject
{
    Q_OBJECT

private:
    explicit Input(uint64_t _amount, QString _address,  QObject *parent = nullptr): QObject(parent), m_amount(_amount), m_pubkey(std::move(_address)) {};

    friend class ConstructionInfo;
    quint64 m_amount;
    QString m_pubkey;

public:
    quint64 amount() const { return m_amount; }
    QString pubKey() const { return m_pubkey; }
};

#endif //FEATHER_INPUT_H
