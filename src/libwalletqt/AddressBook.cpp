// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "AddressBook.h"
#include <QDebug>

AddressBook::AddressBook(Monero::AddressBook *abImpl, QObject *parent)
  : QObject(parent), m_addressBookImpl(abImpl)
{
    getAll();
}

QString AddressBook::errorString() const
{
    return QString::fromStdString(m_addressBookImpl->errorString());
}

int AddressBook::errorCode() const
{
    return m_addressBookImpl->errorCode();
}

void AddressBook::getAll()
{
    emit refreshStarted();

    {
        QWriteLocker locker(&m_lock);

        qDeleteAll(m_rows);

        m_addresses.clear();
        m_rows.clear();

        for (auto &abr: m_addressBookImpl->getAll()) {
            m_addresses.insert(QString::fromStdString(abr->getAddress()), m_rows.size());

            m_rows.append(new AddressBookInfo(abr, this));
        }
    }

    emit refreshFinished();
}

bool AddressBook::getRow(int index, std::function<void (AddressBookInfo &)> callback) const
{
    QReadLocker locker(&m_lock);

    if (index < 0 || index >= m_rows.size())
    {
        return false;
    }

    callback(*m_rows.value(index));
    return true;
}

bool AddressBook::addRow(const QString &address, const QString &payment_id, const QString &description)
{
    //  virtual bool addRow(const std::string &dst_addr , const std::string &payment_id, const std::string &description) = 0;
    bool result;

    {
        QWriteLocker locker(&m_lock);

        result = m_addressBookImpl->addRow(address.toStdString(), payment_id.toStdString(), description.toStdString());
    }

    if (result)
    {
        getAll();
    }

    return result;
}

void AddressBook::setDescription(int index, const QString &description) {
    bool result;

    {
        QWriteLocker locker(&m_lock);

        result = m_addressBookImpl->setDescription(index, description.toStdString());
    }

    if (result)
    {
        getAll();
        emit descriptionChanged();
    }
}

bool AddressBook::deleteRow(int rowId)
{
    bool result;

    {
        QWriteLocker locker(&m_lock);

        result = m_addressBookImpl->deleteRow(rowId);
    }

    // Fetch new data from wallet2.
    if (result)
    {
        getAll();
    }

    return result;
}

quint64 AddressBook::count() const
{
    QReadLocker locker(&m_lock);

    return m_rows.size();
}

QString AddressBook::getDescription(const QString &address) const
{
    QReadLocker locker(&m_lock);

    const QMap<QString, size_t>::const_iterator it = m_addresses.find(address);
    if (it == m_addresses.end())
    {
        return {};
    }
    return m_rows.value(*it)->description();
}

QString AddressBook::getAddress(const QString &description) const
{
    QReadLocker locker(&m_lock);

    for (const auto &row : m_rows) {
        if (row->description() == description) {
            return row->address();
        }
    }

    return QString();
}