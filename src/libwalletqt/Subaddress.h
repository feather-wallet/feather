// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef SUBADDRESS_H
#define SUBADDRESS_H

#include <QObject>
#include <QString>

#include "rows/SubaddressRow.h"

namespace tools {
    class wallet2;
}

class Wallet;
class Subaddress : public QObject
{
    Q_OBJECT

public:
    bool refresh();
    void updateUsed(quint32 accountIndex);
    [[nodiscard]] qsizetype count() const;

    const SubaddressRow& row(int index) const;
    const SubaddressRow& getRow(qsizetype i);
    const QList<SubaddressRow>& getRows();

    bool addRow(const QString &label);
    bool setLabel(quint32 addressIndex, const QString &label);
    bool setHidden(const QString& address, bool hidden);
    bool setPinned(const QString& address, bool pinned);
    bool isHidden(const QString& address);
    bool isPinned(const QString& address);

    QString getError() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void rowUpdated(qsizetype index) const;
    void corrupted() const;
    void noUnusedSubaddresses() const;
    void beginAddRow(qsizetype index) const;
    void endAddRow() const;

private:
    bool emplaceRow(quint32 addressIndex);

    explicit Subaddress(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    Wallet* m_wallet;
    tools::wallet2 *m_wallet2;
    QList<SubaddressRow> m_rows;
    
    QStringList m_pinned;
    QStringList m_hidden;

    QString m_errorString;
};

#endif // SUBADDRESS_H
