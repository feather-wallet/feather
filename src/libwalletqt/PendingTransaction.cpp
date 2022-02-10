// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "PendingTransaction.h"


PendingTransaction::Status PendingTransaction::status() const
{
    return static_cast<Status>(m_pimpl->status());
}

QString PendingTransaction::errorString() const
{
    return QString::fromStdString(m_pimpl->errorString());
}

bool PendingTransaction::commit()
{
    return m_pimpl->commit();
}

bool PendingTransaction::saveToFile(const QString &fileName)
{
    return m_pimpl->commit(fileName.toStdString());
}

quint64 PendingTransaction::amount() const
{
    return m_pimpl->amount();
}

quint64 PendingTransaction::dust() const
{
    return m_pimpl->dust();
}

quint64 PendingTransaction::fee() const
{
    return m_pimpl->fee();
}

QStringList PendingTransaction::txid() const
{
    QStringList list;
    std::vector<std::string> txid = m_pimpl->txid();
    for (const auto &t: txid)
        list.append(QString::fromStdString(t));
    return list;
}


quint64 PendingTransaction::txCount() const
{
    return m_pimpl->txCount();
}

QList<QVariant> PendingTransaction::subaddrIndices() const
{
    std::vector<std::set<uint32_t>> subaddrIndices = m_pimpl->subaddrIndices();
    QList<QVariant> result;
    for (const auto& x : subaddrIndices)
        for (uint32_t i : x)
            result.push_back(i);
    return result;
}

QByteArray PendingTransaction::unsignedTxToBin() const {
    return QByteArray::fromStdString(m_pimpl->unsignedTxToBin());
}

QString PendingTransaction::unsignedTxToBase64() const
{
    return QString::fromStdString(m_pimpl->unsignedTxToBase64());
}

QString PendingTransaction::signedTxToHex(int index) const
{
    return QString::fromStdString(m_pimpl->signedTxToHex(index));
}

PendingTransactionInfo * PendingTransaction::transaction(int index) const {
    return m_pending_tx_info[index];
}

void PendingTransaction::refresh()
{
    qDeleteAll(m_pending_tx_info);
    m_pending_tx_info.clear();

    m_pimpl->refresh();
    for (const auto i : m_pimpl->getAll()) {
        m_pending_tx_info.append(new PendingTransactionInfo(i, this));
    }
}

PendingTransaction::PendingTransaction(Monero::PendingTransaction *pt, QObject *parent)
    : QObject(parent), m_pimpl(pt)
{

}
