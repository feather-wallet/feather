// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "AddressBookModel.h"
#include "AddressBook.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

AddressBookModel::AddressBookModel(QObject *parent, AddressBook *addressBook)
    : QAbstractTableModel(parent)
    , m_addressBook(addressBook)
    , m_showFullAddresses(false)
{
    connect(m_addressBook, &AddressBook::refreshStarted, this, &AddressBookModel::beginResetModel);
    connect(m_addressBook, &AddressBook::refreshFinished, this, &AddressBookModel::endResetModel);
    m_contactIcon = icons()->icon("person.svg");
}

int AddressBookModel::rowCount(const QModelIndex &) const
{
    return m_addressBook->count();
}

int AddressBookModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return COUNT;
}

bool AddressBookModel::isShowFullAddresses() const {
    return m_showFullAddresses;
}

void AddressBookModel::setShowFullAddresses(bool show)
{
    m_showFullAddresses = show;
    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

bool AddressBookModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        const int row = index.row();

        switch (index.column()) {
            case Description:
                m_addressBook->setDescription(row, value.toString());
                break;
            default:
                return false;
        }
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

        return true;
    }
    return false;
}

QVariant AddressBookModel::data(const QModelIndex &index, int role) const
{
    const QList<ContactRow>& rows = m_addressBook->getRows();
    if (index.row() < 0 || index.row() >= rows.size()) {
        return {};
    }
    const ContactRow& row = rows[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
        switch (index.column()) {
            case Address:
            {
                QString address = row.address;
                if (!m_showFullAddresses && role != Qt::UserRole) {
                    address = Utils::displayAddress(address);
                }
                return address;
            }
            case Description:
                return row.label;
            default:
                qCritical() << "Invalid column" << index.column();
        }
    }
    else if (role == Qt::FontRole) {
        switch (index.column()) {
            case Address:
                return Utils::getMonospaceFont();
            default:
                return {};
        }
    }
    else if (role == Qt::DecorationRole) {
        switch (index.column()) {
            case Description: {
                return QVariant(m_contactIcon);  // @TODO: does not actually work
            }
            default: {
                return {};
            }
        }
    }
    return {};
}

Qt::ItemFlags AddressBookModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Description)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}

QVariant AddressBookModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return {};
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Address:
                return QString("Address");
            case Description:
                return QString("Name");
            default:
                return {};
        }
    }
    return {};
}

bool AddressBookModel::deleteRow(int row)
{
    return m_addressBook->deleteRow(row);
}

bool AddressBookModel::writeCSV(const QString &path) {
    QString csv = "";
    for(int i = 0; i < this->rowCount(); i++) {
        QModelIndex index = this->index(i, 0);
        const auto description = this->data(index.siblingAtColumn(AddressBookModel::Description), Qt::UserRole).toString().replace("\"", "");
        const auto address = this->data(index.siblingAtColumn(AddressBookModel::Address), Qt::UserRole).toString();
        if(address.isEmpty()) continue;
        csv += QString("%1,\"%2\"\n").arg(address).arg(description);
    }
    if(csv.isEmpty())
        return false;
    csv = QString("address,description\n%1").arg(csv);
    return Utils::fileWrite(path, csv);
}

QMap<QString, QString> AddressBookModel::readCSV(const QString &path) {
    if(!Utils::fileExists(path)) {
        return QMap<QString, QString>();
    }
    QString csv = Utils::barrayToString(Utils::fileOpen(path));
    QTextStream stream(&csv);
    QMap<QString, QString> map;

    while(!stream.atEnd()) {
        QStringList line = stream.readLine().split(",");
        if(line.length() != 2) {
            continue;
        }
        QString address = line.at(0);
        QString description = line.at(1);
        description = description.replace("\"", "");
        if(!description.isEmpty() && !address.isEmpty()) {
            map[description] = address;
        }
    }
    return map;
}
