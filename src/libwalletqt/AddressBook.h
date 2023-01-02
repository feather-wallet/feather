// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H

#include <wallet/api/wallet2_api.h>
#include "AddressBookInfo.h"
#include <QMap>
#include <QObject>
#include <QReadWriteLock>
#include <QList>
#include <QDateTime>

namespace Monero {
struct AddressBook;
}
class AddressBookRow;

class AddressBook : public QObject
{
    Q_OBJECT
public:
    Q_INVOKABLE bool getRow(int index, std::function<void (AddressBookInfo &)> callback) const;
    Q_INVOKABLE bool addRow(const QString &address, const QString &payment_id, const QString &description);
    Q_INVOKABLE bool deleteRow(int rowId);
    Q_INVOKABLE void setDescription(int index, const QString &label);
    quint64 count() const;
    Q_INVOKABLE QString errorString() const;
    Q_INVOKABLE int errorCode() const;
    Q_INVOKABLE QString getDescription(const QString &address) const;
    Q_INVOKABLE QString getAddress(const QString &description) const;

    enum ErrorCode {
        Status_Ok,
        General_Error,
        Invalid_Address,
        Invalid_Payment_Id
    };

    Q_ENUM(ErrorCode);

private:
    void getAll();

signals:
    void refreshStarted() const;
    void refreshFinished() const;
    void descriptionChanged() const;

private:
    explicit AddressBook(Monero::AddressBook * abImpl, QObject *parent);
    friend class Wallet;
    Monero::AddressBook * m_addressBookImpl;
    mutable QReadWriteLock m_lock;
    QList<AddressBookInfo*> m_rows;
    QMap<QString, size_t> m_addresses;
};

#endif // ADDRESSBOOK_H
