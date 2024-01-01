// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "UnsignedTransaction.h"
#include <QDebug>

UnsignedTransaction::Status UnsignedTransaction::status() const
{
    return static_cast<Status>(m_pimpl->status());
}

QString UnsignedTransaction::errorString() const
{
    return QString::fromStdString(m_pimpl->errorString());
}

quint64 UnsignedTransaction::amount(size_t index) const
{
    std::vector<uint64_t> arr = m_pimpl->amount();
    if(index > arr.size() - 1)
        return 0;
    return arr[index];
}

quint64 UnsignedTransaction::fee(size_t index) const
{
    std::vector<uint64_t> arr = m_pimpl->fee();
    if(index > arr.size() - 1)
        return 0;
    return arr[index];
}

quint64 UnsignedTransaction::mixin(size_t index) const
{
    std::vector<uint64_t> arr = m_pimpl->mixin();
    if(index > arr.size() - 1)
        return 0;
    return arr[index];
}

quint64 UnsignedTransaction::txCount() const
{
    return m_pimpl->txCount();
}

quint64 UnsignedTransaction::minMixinCount() const
{
    return m_pimpl->minMixinCount();
}

QString UnsignedTransaction::confirmationMessage() const
{
    return QString::fromStdString(m_pimpl->confirmationMessage());
}

QStringList UnsignedTransaction::paymentId() const
{
    QList<QString> list;
    for (const auto &t: m_pimpl->paymentId())
        list.append(QString::fromStdString(t));
    return list;
}

QStringList UnsignedTransaction::recipientAddress() const
{
    QList<QString> list;
    for (const auto &t: m_pimpl->recipientAddress())
        list.append(QString::fromStdString(t));
    return list;
}

bool UnsignedTransaction::sign(const QString &fileName) const
{
    if(!m_pimpl->sign(fileName.toStdString()))
        return false;
    return true;
}

bool UnsignedTransaction::signToStr(std::string &data) const {
    return m_pimpl->signToStr(data);
}

void UnsignedTransaction::setFilename(const QString &fileName)
{
    m_fileName = fileName;
}

ConstructionInfo * UnsignedTransaction::constructionInfo(int index) const {
    return m_construction_info[index];
}

void UnsignedTransaction::refresh()
{
    qDeleteAll(m_construction_info);
    m_construction_info.clear();

    m_pimpl->refresh();
    for (const auto i : m_pimpl->getAll()) {
        m_construction_info.append(new ConstructionInfo(i, this));
    }
}

UnsignedTransaction::UnsignedTransaction(Monero::UnsignedTransaction *pt, Monero::Wallet *walletImpl, QObject *parent)
    : QObject(parent), m_pimpl(pt), m_walletImpl(walletImpl)
{

}

UnsignedTransaction::~UnsignedTransaction()
{
    delete m_pimpl;
}
