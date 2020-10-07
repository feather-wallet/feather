// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

#ifndef ADDRESSBOOKMODEL_H
#define ADDRESSBOOKMODEL_H

#include <QAbstractTableModel>
#include <QIcon>

class AddressBook;

class AddressBookModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
        Address = 0,
        Description,
        COUNT
    };

    AddressBookModel(QObject *parent, AddressBook * addressBook);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    Q_INVOKABLE bool deleteRow(int row);
    Q_INVOKABLE int lookupPaymentID(const QString &payment_id) const;

    bool isShowFullAddresses() const;
    void setShowFullAddresses(bool show);
    bool writeCSV(const QString &path);

public slots:
    void startReset();
    void endReset();

private:
    AddressBook * m_addressBook;
    QIcon m_contactIcon;
    bool m_showFullAddresses;
};

#endif // ADDRESSBOOKMODEL_H
