// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_ADDRESSBOOK_H
#define FEATHER_ADDRESSBOOK_H

#include <QObject>
#include <QList>

#include "rows/ContactRow.h"

namespace Monero {
struct AddressBook;
}

namespace tools{
    class wallet2;
}

class Wallet;
class AddressBook : public QObject
{
    Q_OBJECT

public:
    enum ErrorCode {
        Status_Ok,
        General_Error,
        Invalid_Address,
        Invalid_Payment_Id
    };
    Q_ENUM(ErrorCode);

    void refresh();
    qsizetype count() const;

    const ContactRow& getRow(qsizetype index);
    const QList<ContactRow>& getRows();

    bool addRow(const QString &address, const QString &description);
    bool setDescription(qsizetype index, const QString &description);
    bool deleteRow(qsizetype index);

    QString errorString() const;
    ErrorCode errorCode() const;

signals:
    void refreshStarted() const;
    void refreshFinished() const;

private:
    explicit AddressBook(tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    tools::wallet2 *m_wallet2;
    QList<ContactRow> m_rows;

    QString m_errorString;
    ErrorCode m_errorCode;
};

#endif // FEATHER_ADDRESSBOOK_H
