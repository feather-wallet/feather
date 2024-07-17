// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PendingTransaction.h"

#include <QVariant>

PendingTransaction::Status PendingTransaction::status() const
{
    return static_cast<Status>(m_pimpl->status());
}

QString PendingTransaction::errorString() const
{
    return QString::fromStdString(m_pimpl->errorString());
}

const std::exception_ptr PendingTransaction::getException() const {
    return m_pimpl->getException();
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

QStringList PendingTransaction::prefixHashes() const
{
    QStringList prefixHashes;
    for (const auto &hash : m_pimpl->prefixHashes()) {
        prefixHashes.append(QString::fromStdString(hash));
    }
    return prefixHashes;
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

std::string PendingTransaction::unsignedTxToBin() const {
    return m_pimpl->unsignedTxToBin();
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

quint32 PendingTransaction::saveToMMS() {
    return m_pimpl->saveToMMS();
}

QStringList PendingTransaction::destinationAddresses(int index) {
    std::vector<std::string> dests = m_pimpl->destinations(index);
    QStringList destinations;
    for (const auto &dest : dests) {
        destinations << QString::fromStdString(dest);
    }

    return destinations;
}

void PendingTransaction::signMultisigTx() {
    m_pimpl->signMultisigTx();
}

quint64 PendingTransaction::signaturesNeeded() {
    return m_pimpl->signaturesNeeded();
}

bool PendingTransaction::enoughMultisigSignatures() {
    return m_pimpl->enoughMultisigSignatures();
}

QStringList PendingTransaction::signersKeys() {
    QStringList keys;
    for (const auto &key : m_pimpl->signersKeys()) {
        keys.append(QString::fromStdString(key));
    }
    return keys;
}

bool PendingTransaction::haveWeSigned() const {
    return m_pimpl->haveWeSigned();
}

bool PendingTransaction::canSign() const {
    return m_pimpl->canSign();
}

PendingTransaction::PendingTransaction(Monero::PendingTransaction *pt, QObject *parent)
    : QObject(parent)
    , m_pimpl(pt)
{

}
