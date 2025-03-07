// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

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

    const QList<ContactRow>& getRows();
    const ContactRow& getRow(qsizetype i);

    bool addRow(const QString &address, const QString &description);
    bool deleteRow(int rowId);
    bool setDescription(int index, const QString &label);
    qsizetype count() const;
    QString errorString() const;
    ErrorCode errorCode() const;

    void refresh();
    void clearRows();

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void descriptionChanged() const;

private:
    explicit AddressBook(tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    tools::wallet2 *m_wallet2;
    QList<ContactRow> m_rows;

    QString m_errorString;
    ErrorCode m_errorCode;
};

#endif // ADDRESSBOOK_H
