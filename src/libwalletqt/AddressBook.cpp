// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AddressBook.h"

#include <wallet/wallet2.h>

AddressBook::AddressBook(Wallet *wallet, tools::wallet2 *wallet2, QObject *parent)
     : QObject(parent)
     , m_wallet(wallet)
     , m_wallet2(wallet2)
{
    this->refresh();
}

QString AddressBook::errorString() const
{
    return m_errorString;
}

AddressBook::ErrorCode AddressBook::errorCode() const
{
    return m_errorCode;
}

void AddressBook::refresh()
{
    emit refreshStarted();

    clearRows();

    // Fetch from Wallet2 and create vector of AddressBookRow objects
    std::vector<tools::wallet2::address_book_row> rows = m_wallet2->get_address_book();
    for (qsizetype i = 0; i < rows.size(); ++i) {
        tools::wallet2::address_book_row *row = &rows.at(i);

        std::string address;
        if (row->m_has_payment_id)
            address = cryptonote::get_account_integrated_address_as_str(m_wallet2->nettype(), row->m_address, row->m_payment_id);
        else
            address = get_account_address_as_str(m_wallet2->nettype(), row->m_is_subaddress, row->m_address);

        auto* abr = new ContactRow{this,
                                   i,
                                   QString::fromStdString(address),
                                   QString::fromStdString(row->m_description)};
        m_rows.push_back(abr);
    }


    emit refreshFinished();
}

bool AddressBook::getRow(int index, std::function<void (ContactRow &)> callback) const
{
    if (index < 0 || index >= m_rows.size())
    {
        return false;
    }

    callback(*m_rows.value(index));
    return true;
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

    bool r =  m_wallet2->add_address_book_row(info.address, info.has_payment_id ? &info.payment_id : nullptr, description.toStdString(), info.is_subaddress);
    if (r)
        refresh();
    else
        m_errorCode = General_Error;
    return r;
}

bool AddressBook::setDescription(int index, const QString &description) {
    m_errorString = "";

    const auto ab = m_wallet2->get_address_book();
    if (index >= ab.size()){
        return false;
    }

    tools::wallet2::address_book_row entry = ab[index];
    entry.m_description = description.toStdString();
    bool r =  m_wallet2->set_address_book_row(index, entry.m_address, entry.m_has_payment_id ? &entry.m_payment_id : nullptr, entry.m_description, entry.m_is_subaddress);
    if (r)
        refresh();
    else
        m_errorCode = General_Error;
    return r;
}

bool AddressBook::deleteRow(int rowId)
{
    bool r = m_wallet2->delete_address_book_row(rowId);
    if (r)
        refresh();
    return r;
}

qsizetype AddressBook::count() const
{
    return m_rows.length();
}

void AddressBook::clearRows()
{
    qDeleteAll(m_rows);
    m_rows.clear();
}