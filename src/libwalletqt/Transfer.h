// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TRANSFER_H
#define TRANSFER_H

#include <QObject>

class Transfer : public QObject
{
    Q_OBJECT

public:
    explicit Transfer(uint64_t amount, QString address, QObject *parent = nullptr)
            : QObject(parent)
            , m_amount(amount)
            , m_address(std::move(address)) {};

    quint64 amount() const { return m_amount; }
    QString address() const { return m_address; }

private:
    friend class TransactionInfo;
    friend class ConstructionInfo;

    quint64 m_amount;
    QString m_address;
};

#endif // TRANSFER_H
