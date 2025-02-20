// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef SUBADDRESS_H
#define SUBADDRESS_H

#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QDateTime>

#include "Wallet.h"
#include "rows/SubaddressRow.h"

namespace tools {
    class wallet2;
}

class Subaddress : public QObject
{
    Q_OBJECT

public:
    bool getRow(int index, std::function<void (SubaddressRow &row)> callback) const;
    bool addRow(quint32 accountIndex, const QString &label);
    
    bool setLabel(quint32 accountIndex, quint32 addressIndex, const QString &label);

    bool setHidden(const QString& address, bool hidden);
    bool isHidden(const QString& address);
    
    bool setPinned(const QString& address, bool pinned);
    bool isPinned(const QString& address);
    
    bool refresh(quint32 accountIndex);
    
    [[nodiscard]] qsizetype count() const;
    void clearRows();

    [[nodiscard]] SubaddressRow* row(int index) const;

    QString getError() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void corrupted() const;

private:
    explicit Subaddress(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    Wallet* m_wallet;
    tools::wallet2 *m_wallet2;
    QList<SubaddressRow*> m_rows;
    
    QStringList m_pinned;
    QStringList m_hidden;

    QString m_errorString;
};

#endif // SUBADDRESS_H
