// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#ifndef FEATHER_ADDRESSBOOKINFO_H
#define FEATHER_ADDRESSBOOKINFO_H

#include <wallet/api/wallet2_api.h>
#include <QObject>

class AddressBookInfo : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString address READ address);
    Q_PROPERTY(QString description READ description);

public:
    QString address() const;
    QString description() const;

private:
    explicit AddressBookInfo(const Monero::AddressBookRow *pimpl, QObject *parent = nullptr);

    friend class AddressBook;
    QString m_address;
    QString m_description;
};


#endif //FEATHER_ADDRESSBOOKINFO_H
