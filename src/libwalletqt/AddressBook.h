// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <wallet/api/wallet2_api.h>
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QDateTime>

#include "rows/ContactRow.h"
#include "Wallet.h"

namespace Monero {
struct AddressBook;
}

namespace tools{
    class wallet2;
}

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

    bool getRow(int index, std::function<void (ContactRow &)> callback) const;
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
    explicit AddressBook(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent);
    friend class Wallet;

    Wallet *m_wallet;
    tools::wallet2 *m_wallet2;
    QList<ContactRow*> m_rows;

    QString m_errorString;
    ErrorCode m_errorCode;
};

#endif // ADDRESSBOOK_H
