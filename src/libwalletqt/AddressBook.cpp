// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AddressBook.h"

#include <wallet/wallet2.h>

AddressBook::AddressBook(tools::wallet2 *wallet2, QObject *parent)
    : QObject(parent)
    , m_wallet2(wallet2)
    , m_errorCode(Status_Ok)
{
    this->refresh();
}

void AddressBook::refresh()
{
    emit refreshStarted();

    m_rows.clear();

    for (const auto &row : m_wallet2->get_address_book()) {
        std::string address;
        if (row.m_has_payment_id)
            address = cryptonote::get_account_integrated_address_as_str(m_wallet2->nettype(), row.m_address, row.m_payment_id);
        else
            address = get_account_address_as_str(m_wallet2->nettype(), row.m_is_subaddress, row.m_address);

        m_rows.emplaceBack(QString::fromStdString(address), QString::fromStdString(row.m_description));
    }

    emit refreshFinished();
}

qsizetype AddressBook::count() const
{
    return m_rows.length();
}

const ContactRow& AddressBook::getRow(const qsizetype index)
{
    if (index < 0 || index >= m_rows.size()) {
        throw std::out_of_range("Index out of range");
    }
    return m_rows[index];
}

const QList<ContactRow>& AddressBook::getRows()
{
    return m_rows;
}

bool AddressBook::addRow(const QString &address, const QString &description)
{
    m_errorString = "";

    cryptonote::address_parse_info info;
    if (!cryptonote::get_account_address_from_str(info, m_wallet2->nettype(), address.toStdString())) {
        m_errorString = tr("Invalid destination address");
        m_errorCode = Invalid_Address;
        return false;
    }

    bool r = m_wallet2->add_address_book_row(info.address, info.has_payment_id ? &info.payment_id : nullptr, description.toStdString(), info.is_subaddress);
    if (r)
        refresh();
    else
        m_errorCode = General_Error;
    return r;
}

bool AddressBook::setDescription(qsizetype index, const QString &description) {
    m_errorString = "";

    const auto ab = m_wallet2->get_address_book();
    if (index >= ab.size()){
        return false;
    }

    tools::wallet2::address_book_row entry = ab[index];
    entry.m_description = description.toStdString();
    bool r = m_wallet2->set_address_book_row(index, entry.m_address, entry.m_has_payment_id ? &entry.m_payment_id : nullptr, entry.m_description, entry.m_is_subaddress);
    if (r)
        refresh();
    else
        m_errorCode = General_Error;
    return r;
}

bool AddressBook::deleteRow(qsizetype index)
{
    bool r = m_wallet2->delete_address_book_row(index);
    if (r)
        refresh();
    return r;
}

QString AddressBook::errorString() const
{
    return m_errorString;
}

AddressBook::ErrorCode AddressBook::errorCode() const
{
    return m_errorCode;
}
