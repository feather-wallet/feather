// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TRANSFER_H
#define TRANSFER_H

#include <wallet/api/wallet2_api.h>
#include <QObject>
#include <utility>

class Transfer : public QObject
{
    Q_OBJECT

public:
    explicit Transfer(uint64_t _amount, QString _address,  QObject *parent = 0)
            : QObject(parent), m_amount(_amount), m_address(std::move(_address)) {};
private:
    friend class TransactionInfo;
    friend class ConstructionInfo;
    quint64 m_amount;
    QString m_address;
public:
    quint64 amount() const { return m_amount; }
    QString address() const { return m_address; }

};

#endif // TRANSACTIONINFO_H
