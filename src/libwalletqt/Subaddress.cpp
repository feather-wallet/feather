// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "Subaddress.h"

#include "Wallet.h"
#include <wallet/wallet2.h>

Subaddress::Subaddress(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet(wallet)
    , m_wallet2(wallet2)
{
    QString pinned = m_wallet->getCacheAttribute("feather.pinnedaddresses");
    m_pinned = pinned.split(",");

    QString hidden = m_wallet->getCacheAttribute("feather.hiddenaddresses");
    m_hidden = hidden.split(",");
}

bool Subaddress::refresh(quint32 accountIndex)
{
    emit refreshStarted();

    m_rows.clear();

    bool potentialWalletFileCorruption = false;

    for (quint32 i = 0; i < m_wallet2->get_num_subaddresses(accountIndex); ++i)
    {
        cryptonote::subaddress_index index = {accountIndex, i};
        cryptonote::account_public_address address = m_wallet2->get_subaddress(index);

        // Make sure we have previously generated Di
        auto idx =  m_wallet2->get_subaddress_index(address);
        if (!idx) {
            potentialWalletFileCorruption = true;
            break;
        }

        // Verify mapping
        if (idx != index) {
            potentialWalletFileCorruption = true;
            break;
        }

        QString addressStr = QString::fromStdString(cryptonote::get_account_address_as_str(m_wallet2->nettype(), !index.is_zero(), address));

        m_rows.emplace_back(
            addressStr,
            QString::fromStdString(m_wallet2->get_subaddress_label(index)),
            m_wallet2->get_subaddress_used({accountIndex, (uint32_t)i}),
            this->isHidden(addressStr),
            this->isPinned(addressStr)
        );
    }

    // Make sure keys are intact. We NEVER want to display incorrect addresses in case of memory corruption.
    potentialWalletFileCorruption = potentialWalletFileCorruption || (m_wallet2->get_device_type() == hw::device::SOFTWARE && !m_wallet2->verify_keys());

    if (potentialWalletFileCorruption) {
        LOG_ERROR("KEY INCONSISTENCY DETECTED, WALLET IS IN CORRUPT STATE.");
        m_rows.clear();
        emit corrupted();
    }

    emit refreshFinished();

    return !potentialWalletFileCorruption;
}

void Subaddress::updateUsed(quint32 accountIndex)
{
    for (quint32 i = 0; i < m_rows.count(); i++) {
        SubaddressRow& row = m_rows[i];

        if (m_wallet2->get_subaddress_used({accountIndex, i}) != row.used) {
            row.used = !row.used;
            emit rowUpdated(i);
        }
    }
}

qsizetype Subaddress::count() const
{
    return m_rows.length();
}

const SubaddressRow& Subaddress::row(int index) const
{
    return m_rows[index];
}

const SubaddressRow& Subaddress::getRow(const qsizetype i)
{
    if (i < 0 || i >= m_rows.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_rows[i];
}

const QList<SubaddressRow>& Subaddress::getRows()
{
    return m_rows;
}

bool Subaddress::addRow(quint32 accountIndex, const QString &label)
{
    // This can fail if hardware device is unplugged during operating, catch here to prevent crash
    // Todo: Notify GUI that it was a device error
    try
    {
        m_wallet2->add_subaddress(accountIndex, label.toStdString());
        refresh(accountIndex);
    }
    catch (const std::exception& e)
    {
        if (m_wallet2->key_on_device()) {
        }
        m_errorString = QString::fromStdString(e.what());
        return false;
    }

    return true;
}

bool Subaddress::setLabel(quint32 accountIndex, quint32 addressIndex, const QString &label)
{
    try {
        m_wallet2->set_subaddress_label({accountIndex, addressIndex}, label.toStdString());
        refresh(accountIndex);
    }
    catch (const std::exception& e)
    {
        return false;
    }

    return true;
}

bool Subaddress::setHidden(const QString &address, bool hidden) 
{
    if (hidden) {
        if (m_hidden.contains(address)) {
            return false;
        }
        m_hidden.append(address);
    }
    else {
        if (!m_hidden.contains(address)) {
            return false;
        }
        m_hidden.removeAll(address);
    }
    
    bool r = m_wallet->setCacheAttribute("feather.hiddenaddresses", m_hidden.join(","));
    
    refresh(m_wallet->currentSubaddressAccount());
    return r;
}

bool Subaddress::setPinned(const QString &address, bool pinned)
{
    if (pinned) {
        if (m_pinned.contains(address)) {
            return false;
        }
        m_pinned.append(address);
    }
    else {
        if (!m_pinned.contains(address)) {
            return false;
        }
        m_pinned.removeAll(address);
    }

    bool r = m_wallet->setCacheAttribute("feather.pinnedaddresses", m_pinned.join(","));

    refresh(m_wallet->currentSubaddressAccount());
    return r;
}

bool Subaddress::isHidden(const QString &address) 
{
    return m_hidden.contains(address);
}

bool Subaddress::isPinned(const QString &address)
{
    return m_pinned.contains(address);
}

QString Subaddress::getError() const {
    return m_errorString;
}
