// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "TransactionHistory.h"
#include "TransactionInfo.h"
#include "utils/utils.h"
#include "utils/AppData.h"
#include "utils/config.h"

bool TransactionHistory::transaction(int index, std::function<void (TransactionInfo &)> callback)
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_tinfo.size()) {
        qCritical("%s: no transaction info for index %d", __FUNCTION__, index);
        qCritical("%s: there's %d transactions in backend", __FUNCTION__, m_pimpl->count());
        return false;
    }

    callback(*m_tinfo.value(index));
    return true;
}

TransactionInfo* TransactionHistory::transaction(const QString &id)
{
    QReadLocker locker(&m_lock);

    auto itr = std::find_if(m_tinfo.begin(), m_tinfo.end(),
            [&](const TransactionInfo * ti) {
        return ti->hash() == id;
    });
    return itr != m_tinfo.end() ? *itr : nullptr;
}

TransactionInfo* TransactionHistory::transaction(int index)
{
    if (index < 0 || index >= m_tinfo.size()) {
        return nullptr;
    }

    return m_tinfo[index];
}

void TransactionHistory::refresh(quint32 accountIndex)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QDateTime firstDateTime = QDate(2014, 4, 18).startOfDay();
#else
    QDateTime firstDateTime = QDateTime(QDate(2014, 4, 18)); // the genesis block
#endif
    QDateTime lastDateTime  = QDateTime::currentDateTime().addDays(1); // tomorrow (guard against jitter and timezones)

    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        qDeleteAll(m_tinfo);
        m_tinfo.clear();

        quint64 lastTxHeight = 0;
        m_locked = false;
        m_minutesToUnlock = 0;

        m_pimpl->refresh();
        for (const auto i : m_pimpl->getAll()) {
            if (i->subaddrAccount() != accountIndex) {
                continue;
            }

            m_tinfo.append(new TransactionInfo(i, this));

            const TransactionInfo *ti = m_tinfo.back();
            // looking for transactions timestamp scope
            if (ti->timestamp() >= lastDateTime) {
                lastDateTime = ti->timestamp();
            }
            if (ti->timestamp() <= firstDateTime) {
                firstDateTime = ti->timestamp();
            }
            quint64 requiredConfirmations = (ti->blockHeight() < ti->unlockTime()) ? ti->unlockTime() - ti->blockHeight() : 10;
            // store last tx height
            if (ti->confirmations() < requiredConfirmations && ti->blockHeight() >= lastTxHeight) {
                lastTxHeight = ti->blockHeight();
                // TODO: Fetch block time and confirmations needed from wallet2?
                m_minutesToUnlock = (requiredConfirmations - ti->confirmations()) * 2;
                m_locked = true;
            }
        }
    }

    emit refreshFinished();

    if (m_firstDateTime != firstDateTime) {
        m_firstDateTime = firstDateTime;
        emit firstDateTimeChanged();
    }
    if (m_lastDateTime != lastDateTime) {
        m_lastDateTime = lastDateTime;
        emit lastDateTimeChanged();
    }
}

void TransactionHistory::setTxNote(const QString &txid, const QString &note)
{
    m_pimpl->setTxNote(txid.toStdString(), note.toStdString());
    this->refresh(0); // todo: get actual account index
    emit txNoteChanged();
}

quint64 TransactionHistory::count() const
{
    QReadLocker locker(&m_lock);

    return m_tinfo.count();
}

QDateTime TransactionHistory::firstDateTime() const
{
    return m_firstDateTime;
}

QDateTime TransactionHistory::lastDateTime() const
{
    return m_lastDateTime;
}

quint64 TransactionHistory::minutesToUnlock() const
{
    return m_minutesToUnlock;
}

bool TransactionHistory::TransactionHistory::locked() const
{
    return m_locked;
}


TransactionHistory::TransactionHistory(Monero::TransactionHistory *pimpl, QObject *parent)
    : QObject(parent), m_pimpl(pimpl), m_minutesToUnlock(0), m_locked(false)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_firstDateTime = QDate(2014, 4, 18).startOfDay();
#else
    m_firstDateTime  = QDateTime(QDate(2014, 4, 18)); // the genesis block
#endif
    m_lastDateTime = QDateTime::currentDateTime().addDays(1); // tomorrow (guard against jitter and timezones)
}

bool TransactionHistory::writeCSV(const QString &path) {
    auto data = QString("");
    QReadLocker locker(&m_lock);
    for (const auto &tx : m_pimpl->getAll()) {
        if (tx->subaddrAccount() != 0) { // only account 0
            continue;
        }

        TransactionInfo info(tx, this);

        // collect column data
        QDateTime timeStamp = info.timestamp();
        double amount = info.amount();

        // calc historical fiat price
        QString fiatAmount;
        QString preferredFiatSymbol = config()->get(Config::preferredFiatCurrency).toString();
        const double usd_price = appData()->txFiatHistory->get(timeStamp.toString("yyyyMMdd"));
        double fiat_price = usd_price * amount;

        if(preferredFiatSymbol != "USD")
            fiat_price = appData()->prices.convert("USD", preferredFiatSymbol, fiat_price);
        double fiat_rounded = ceil(Utils::roundSignificant(fiat_price, 3) * 100.0) / 100.0;
        if(fiat_price != 0)
            fiatAmount = QString("%1 %2").arg(QString::number(fiat_rounded)).arg(preferredFiatSymbol);

        // collect some more column data
        quint64 atomicAmount = info.atomicAmount();
        quint32 subaddrAccount = info.subaddrAccount();
        QString fee = info.fee();
        QString direction = QString("");
        TransactionInfo::Direction _direction = info.direction();
        if(_direction == TransactionInfo::Direction_In)
            direction = QString("in");
        else if(_direction == TransactionInfo::Direction_Out)
            direction = QString("out");
        else
            continue;  // skip TransactionInfo::Direction_Both

        QString label = info.label();
        label.remove(QChar('"'));  // reserved
        quint64 blockHeight = info.blockHeight();
        QString date = info.date() + " " + info.time();
        uint epoch = timeStamp.toTime_t();
        QString displayAmount = info.displayAmount();
        QString paymentId = info.paymentId();
        if(paymentId == "0000000000000000")
            paymentId = "";

        // format and write
        QString line = QString("%1,%2,%3,%4,%5,\"%6\",%7,%8,%9,\"%10\",%11,%12\n")
            .arg(QString::number(blockHeight), QString::number(epoch), date)
            .arg(direction, QString::number(amount), fiatAmount, QString::number(atomicAmount))
            .arg(info.fee(), info.hash(), label, QString::number(subaddrAccount))
            .arg(paymentId);
        data += line;
    }

    if(data.isEmpty())
        return false;

    data = QString("blockHeight,epoch,date,direction,amount,fiat,atomicAmount,fee,txid,label,subaddrAccount,paymentId\n%1").arg(data);
    return Utils::fileWrite(path, data);
}
