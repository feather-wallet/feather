// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_ACCOUNTROW_H
#define FEATHER_ACCOUNTROW_H

#include <QObject>

class AccountRow : public QObject
{
Q_OBJECT

public:
    AccountRow(QObject *parent, qsizetype row, const QString& address, const QString &label, uint64_t balance, uint64_t unlockedBalance)
            : QObject(parent)
            , m_row(row)
            , m_address(address)
            , m_label(label)
            , m_balance(balance)
            , m_unlockedBalance(unlockedBalance) {}

    qsizetype getRow() const;
    const QString& getAddress() const;
    const QString& getLabel() const;
    QString getBalance() const;
    QString getUnlockedBalance() const;

private:
    qsizetype m_row;
    QString m_address;
    QString m_label;
    uint64_t m_balance;
    uint64_t m_unlockedBalance;
};

#endif //FEATHER_ACCOUNTROW_H
