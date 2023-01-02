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
        return m_coinbase ? "Coinbase" : "Change";
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

CoinsInfo::CoinsInfo(const Monero::CoinsInfo *pimpl, QObject *parent)
        : QObject(parent)
        , m_blockHeight(pimpl->blockHeight())
        , m_hash(QString::fromStdString(pimpl->hash()))
        , m_internalOutputIndex(pimpl->internalOutputIndex())
        , m_globalOutputIndex(pimpl->globalOutputIndex())
        , m_spent(pimpl->spent())
        , m_frozen(pimpl->frozen())
        , m_spentHeight(pimpl->spentHeight())
        , m_amount(pimpl->amount())
        , m_rct(pimpl->rct())
        , m_keyImageKnown(pimpl->keyImageKnown())
        , m_pkIndex(pimpl->pkIndex())
        , m_subaddrIndex(pimpl->subaddrIndex())
        , m_subaddrAccount(pimpl->subaddrAccount())
        , m_address(QString::fromStdString(pimpl->address()))
        , m_addressLabel(QString::fromStdString(pimpl->addressLabel()))
        , m_keyImage(QString::fromStdString(pimpl->keyImage()))
        , m_unlockTime(pimpl->unlockTime())
        , m_unlocked(pimpl->unlocked())
        , m_pubKey(QString::fromStdString(pimpl->pubKey()))
        , m_coinbase(pimpl->coinbase())
        , m_description(QString::fromStdString(pimpl->description()))
{

}
