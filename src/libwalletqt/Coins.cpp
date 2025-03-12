// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "Coins.h"
#include "rows/CoinsInfo.h"
#include "Wallet.h"
#include <wallet/wallet2.h>

Coins::Coins(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
        : QObject(parent)
        , m_wallet(wallet)
        , m_wallet2(wallet2)
{

}

void Coins::refresh()
{
    qDebug() << Q_FUNC_INFO;

    emit refreshStarted();

    boost::shared_lock<boost::shared_mutex> transfers_lock(m_wallet2->m_transfers_mutex);

    m_rows.clear();
    for (size_t i = 0; i < m_wallet2->get_num_transfer_details(); ++i)
    {
        const tools::wallet2::transfer_details &td = m_wallet2->get_transfer_details(i);

        if (td.m_subaddr_index.major != m_wallet->currentSubaddressAccount()) {
            continue;
        }

        CoinsInfo ci;
        ci.blockHeight = td.m_block_height;
        ci.hash = QString::fromStdString(epee::string_tools::pod_to_hex(td.m_txid));
        ci.internalOutputIndex = td.m_internal_output_index;
        ci.globalOutputIndex = td.m_global_output_index;
        ci.spent = td.m_spent;
        ci.frozen = td.m_frozen;
        ci.spentHeight = td.m_spent_height;
        ci.amount = td.m_amount;
        ci.rct = td.m_rct;
        ci.keyImageKnown = td.m_key_image_known;
        ci.pkIndex = td.m_pk_index;
        ci.subaddrIndex = td.m_subaddr_index.minor;
        ci.subaddrAccount = td.m_subaddr_index.major;
        ci.address = QString::fromStdString(m_wallet2->get_subaddress_as_str(td.m_subaddr_index)); // todo: this is expensive, cache maybe?
        ci.addressLabel = QString::fromStdString(m_wallet2->get_subaddress_label(td.m_subaddr_index));
        ci.txNote = QString::fromStdString(m_wallet2->get_tx_note(td.m_txid));
        ci.keyImage = QString::fromStdString(epee::string_tools::pod_to_hex(td.m_key_image));
        ci.unlockTime = td.m_tx.unlock_time;
        ci.unlocked = m_wallet2->is_transfer_unlocked(td);
        ci.pubKey = QString::fromStdString(epee::string_tools::pod_to_hex(td.get_public_key()));
        ci.coinbase = td.m_tx.vin.size() == 1 && td.m_tx.vin[0].type() == typeid(cryptonote::txin_gen);
        ci.description = m_wallet->getCacheAttribute(QString("coin.description:%1").arg(ci.pubKey));
        ci.change = m_wallet2->is_change(td);

        m_rows.push_back(ci);
    }

    emit refreshFinished();
}

quint64 Coins::count() const
{
    return m_rows.length();
}

const CoinsInfo& Coins::getRow(const qsizetype i)
{
    if (i < 0 || i >= m_rows.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_rows[i];
}

const QList<CoinsInfo>& Coins::getRows()
{
    return m_rows;
}

void Coins::setDescription(const QString &publicKey, quint32 accountIndex, const QString &description)
{
    m_wallet->setCacheAttribute(QString("coin.description:%1").arg(publicKey), description);
    this->refresh();
    emit descriptionChanged();
}

void Coins::freeze(QStringList &publicKeys)
{
    crypto::public_key pk;

    for (const auto& publicKey : publicKeys) {
        if (!epee::string_tools::hex_to_pod(publicKey.toStdString(), pk))
        {
            qWarning() << "Invalid public key: " << publicKey;
            continue;
        }

        try
        {
            m_wallet2->freeze(pk);
        }
        catch (const std::exception& e)
        {
            qWarning() << "freeze: " << e.what();
        }
    }

    refresh();
}

void Coins::thaw(QStringList &publicKeys)
{
    crypto::public_key pk;

    for (const auto& publicKey : publicKeys) {
        if (!epee::string_tools::hex_to_pod(publicKey.toStdString(), pk))
        {
            qWarning() << "Invalid public key: " << publicKey;
            continue;
        }

        try
        {
            m_wallet2->thaw(pk);
        }
        catch (const std::exception& e)
        {
            qWarning() << "thaw: " << e.what();
        }
    }

    refresh();
}

quint64 Coins::sumAmounts(const QStringList &keyImages) {
    quint64 amount = 0;
    for (const CoinsInfo& coin : m_rows) {
        if (!coin.keyImageKnown) {
            continue;
        }

        if (!keyImages.contains(coin.keyImage)) {
            continue;
        }

        amount += coin.amount;
    }

    return amount;
}
