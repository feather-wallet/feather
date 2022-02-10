// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "AddressBookInfo.h"

QString AddressBookInfo::address() const {
    return m_address;
}

QString AddressBookInfo::description() const {
    return m_description;
}

AddressBookInfo::AddressBookInfo(const Monero::AddressBookRow *pimpl, QObject *parent)
        : QObject(parent)
        , m_address(QString::fromStdString(pimpl->getAddress()))
        , m_description(QString::fromStdString(pimpl->getDescription()))
{

}
