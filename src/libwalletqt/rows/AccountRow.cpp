// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AccountRow.h"
#include "WalletManager.h"

qsizetype AccountRow::getRow() const {
    return m_row;
}

const QString& AccountRow::getAddress() const {
    return m_address;
}

const QString& AccountRow::getLabel() const {
    return m_label;
}

QString AccountRow::getBalance() const {
    return WalletManager::displayAmount(m_balance);
}

QString AccountRow::getUnlockedBalance() const {
    return WalletManager::displayAmount(m_unlockedBalance);
}