// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TransactionRow.h"
#include "WalletManager.h"
#include "Output.h"

TransactionRow::TransactionRow()
        : amount(0)
        , balanceDelta(0)
        , blockHeight(0)
        , confirmations(0)
        , direction(TransactionRow::Direction_Out)
        , subaddrAccount(0)
        , unlockTime(0)
        , failed(false)
        , pending(false)
        , coinbase(false)
        , fee(0)
{
}

double TransactionRow::amountDouble() const
{
    return displayAmount().toDouble();
}

QString TransactionRow::displayAmount() const
{
    return WalletManager::displayAmount(amount);
}

QString TransactionRow::displayFee() const
{
    if (fee == 0)
        return "";
    return WalletManager::displayAmount(fee);
}

quint64 TransactionRow::confirmationsRequired() const
{
    return (blockHeight < unlockTime) ? unlockTime - blockHeight : 10;
}

QString TransactionRow::date() const
{
    return timestamp.date().toString(Qt::ISODate);
}

QString TransactionRow::time() const
{
    return timestamp.time().toString(Qt::ISODate);
}

QList<QString> TransactionRow::destinations() const
{
    QList<QString> dests;
    for (auto const& t: transfers) {
        dests.append(t.address);
    }
    return dests;
}

QString TransactionRow::rings_formatted() const
{
    QString ringsStr;
    for (auto const& r: rings) {
        ringsStr += r.keyImage + ": \n";
        for (uint64_t m : r.ringMembers){
            ringsStr += QString::number(m) + " ";
        }
        ringsStr += "\n\n";
    }
    return ringsStr;
}

bool TransactionRow::hasPaymentId() const {
    return paymentId != "0000000000000000";
}
