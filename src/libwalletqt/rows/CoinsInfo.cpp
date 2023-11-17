// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "CoinsInfo.h"
#include "libwalletqt/WalletManager.h"

quint64 CoinsInfo::blockHeight() const
{
    return m_blockHeight;
}

QString CoinsInfo::hash() const
{
    return m_hash;
}

quint64 CoinsInfo::internalOutputIndex() const
{
    return m_internalOutputIndex;
}

quint64 CoinsInfo::globalOutputIndex() const
{
    return m_globalOutputIndex;
}

bool CoinsInfo::spent() const
{
    return m_spent;
}

bool CoinsInfo::frozen() const
{
    return m_frozen;
}

quint64 CoinsInfo::spentHeight() const
{
    return m_spentHeight;
}

quint64 CoinsInfo::amount() const {
    return m_amount;
}

QString CoinsInfo::displayAmount() const
{
    return WalletManager::displayAmount(m_amount);
}

bool CoinsInfo::rct() const {
    return m_rct;
}

bool CoinsInfo::keyImageKnown() const {
    return m_keyImageKnown;
}

quint64 CoinsInfo::pkIndex() const {
    return m_pkIndex;
}

quint32 CoinsInfo::subaddrIndex() const {
    return m_subaddrIndex;
}

quint32 CoinsInfo::subaddrAccount() const {
    return m_subaddrAccount;
}

QString CoinsInfo::address() const {
    return m_address;
}

QString CoinsInfo::addressLabel() const {
    if (m_subaddrIndex == 0) {
        if (m_coinbase) {
            return "Coinbase";
        }
        if (m_change) {
            return "Change";
        }
        if (m_addressLabel == "Primary account") {
            return "Primary address";
        }
    }

    return m_addressLabel;
}

QString CoinsInfo::keyImage() const {
    return m_keyImage;
}

quint64 CoinsInfo::unlockTime() const {
    return m_unlockTime;
}

bool CoinsInfo::unlocked() const {
    return m_unlocked;
}

void CoinsInfo::setUnlocked(bool unlocked) {
    m_unlocked = unlocked;
}

QString CoinsInfo::pubKey() const {
    return m_pubKey;
}

bool CoinsInfo::coinbase() const {
    return m_coinbase;
}

QString CoinsInfo::description() const {
    return m_description;
}

bool CoinsInfo::change() const {
    return m_change;
}

CoinsInfo::CoinsInfo(QObject *parent)
        : QObject(parent)
        , m_blockHeight(0)
        , m_internalOutputIndex(0)
        , m_globalOutputIndex(0)
        , m_spent(false)
        , m_frozen(false)
        , m_spentHeight(0)
        , m_amount(0)
        , m_rct(false)
        , m_keyImageKnown(false)
        , m_pkIndex(0)
        , m_subaddrIndex(0)
        , m_subaddrAccount(0)
        , m_unlockTime(0)
        , m_unlocked(false)
        , m_coinbase(false)
        , m_change(false)
{

}
