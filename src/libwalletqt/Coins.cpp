// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Coins.h"
#include "rows/CoinsInfo.h"
#include <wallet/wallet2.h>

Coins::Coins(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
        : QObject(parent)
        , m_wallet(wallet)
        , m_wallet2(wallet2)
{

}

bool Coins::coin(int index, std::function<void (CoinsInfo &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %lld transactions in backend", __FUNCTION__, m_rows.count());
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

CoinsInfo* Coins::coin(int index)
{
    return m_rows.value(index);
}

void Coins::refresh()
{
    emit refreshStarted();

    boost::shared_lock<boost::shared_mutex> transfers_lock(m_wallet2->m_transfers_mutex);

    {
        QWriteLocker locker(&m_lock);

        clearRows();
        uint32_t account = m_wallet->currentSubaddressAccount();

        for (size_t i = 0; i < m_wallet2->get_num_transfer_details(); ++i)
        {
            const tools::wallet2::transfer_details &td = m_wallet2->get_transfer_details(i);

            if (td.m_subaddr_index.major != account) {
                continue;
            }

            auto ci = new CoinsInfo(this);
            ci->m_blockHeight = td.m_block_height;
            ci->m_hash = QString::fromStdString(epee::string_tools::pod_to_hex(td.m_txid));
            ci->m_internalOutputIndex = td.m_internal_output_index;
            ci->m_globalOutputIndex = td.m_global_output_index;
            ci->m_spent = td.m_spent;
            ci->m_frozen = td.m_frozen;
            ci->m_spentHeight = td.m_spent_height;
            ci->m_amount = td.m_amount;
            ci->m_rct = td.m_rct;
            ci->m_keyImageKnown = td.m_key_image_known;
            ci->m_pkIndex = td.m_pk_index;
            ci->m_subaddrIndex = td.m_subaddr_index.minor;
            ci->m_subaddrAccount = td.m_subaddr_index.major;
            ci->m_address = QString::fromStdString(m_wallet2->get_subaddress_as_str(td.m_subaddr_index)); // todo: this is expensive, cache maybe?
            ci->m_addressLabel = QString::fromStdString(m_wallet2->get_subaddress_label(td.m_subaddr_index));
            ci->m_txNote = QString::fromStdString(m_wallet2->get_tx_note(td.m_txid));
            ci->m_keyImage = QString::fromStdString(epee::string_tools::pod_to_hex(td.m_key_image));
            ci->m_unlockTime = td.m_tx.unlock_time;
            ci->m_unlocked = m_wallet2->is_transfer_unlocked(td);
            ci->m_pubKey = QString::fromStdString(epee::string_tools::pod_to_hex(td.get_public_key()));
            ci->m_coinbase = td.m_tx.vin.size() == 1 && td.m_tx.vin[0].type() == typeid(cryptonote::txin_gen);
            ci->m_description = m_wallet->getCacheAttribute(QString("coin.description:%1").arg(ci->m_pubKey));
            ci->m_change = m_wallet2->is_change(td);

            m_rows.push_back(ci);
        }
    }

    emit refreshFinished();
}

void Coins::refreshUnlocked()
{
    QWriteLocker locker(&m_lock);

    for (CoinsInfo* c : m_rows) {
        if (!c->unlocked()) {
            bool unlocked = m_wallet2->is_transfer_unlocked(c->unlockTime(), c->blockHeight());
            c->setUnlocked(unlocked);
        }
    }
}

quint64 Coins::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.length();
}

void Coins::freeze(QString &publicKey)
{
    crypto::public_key pk;
    if (!epee::string_tools::hex_to_pod(publicKey.toStdString(), pk))
    {
        qWarning() << "Invalid public key: " << publicKey;
        return;
    }

    try
    {
        m_wallet2->freeze(pk);
        refresh();
    }
    catch (const std::exception& e)
    {
        qWarning() << "freeze: " << e.what();
    }

    emit coinFrozen();
}

void Coins::thaw(QString &publicKey)
{
    crypto::public_key pk;
    if (!epee::string_tools::hex_to_pod(publicKey.toStdString(), pk))
    {
        qWarning() << "Invalid public key: " << publicKey;
        return;
    }

    try
    {
        m_wallet2->thaw(pk);
        refresh();
    }
    catch (const std::exception& e)
    {
        qWarning() << "thaw: " << e.what();
    }

    emit coinThawed();
}

QVector<CoinsInfo*> Coins::coins_from_txid(const QString &txid)
{
    QVector<CoinsInfo*> coins;

    for (int i = 0; i < this->count(); i++) {
        CoinsInfo* coin = this->coin(i);
        if (coin->hash() == txid) {
            coins.append(coin);
        }
    }
    return coins;
}

QVector<CoinsInfo*> Coins::coinsFromKeyImage(const QStringList &keyimages) {
    QVector<CoinsInfo*> coins;

    for (int i = 0; i < this->count(); i++) {
        CoinsInfo* coin = this->coin(i);
        if (coin->keyImageKnown() && keyimages.contains(coin->keyImage())) {
            coins.append(coin);
        }
    }

    return coins;
}

void Coins::setDescription(const QString &publicKey, quint32 accountIndex, const QString &description)
{
    m_wallet->setCacheAttribute(QString("coin.description:%1").arg(publicKey), description);
    this->refresh();
    emit descriptionChanged();
}

void Coins::clearRows() {
    qDeleteAll(m_rows);
    m_rows.clear();
}