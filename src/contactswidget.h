// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef CONTACTSWIDGET_H
#define CONTACTSWIDGET_H

#include "model/AddressBookModel.h"
#include "model/AddressBookProxyModel.h"
#include "appcontext.h"

#include <QWidget>
#include <QMenu>

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsWidget(QWidget *parent = nullptr);
    void setModel(AddressBookModel *model, Wallet *wallet);
    ~ContactsWidget() override;

public slots:
    void copyAddress();
    void copyName();
    void payTo();
    void newContact(QString address = "", QString name = "");
    void deleteContact();
    void setShowFullAddresses(bool show);
    void setSearchFilter(const QString &filter);
    void resetModel();

signals:
    void fillAddress(QString &address);

private slots:
    void showHeaderMenu(const QPoint &position);

private:
    Ui::ContactsWidget *ui;

    QAction *m_showFullAddressesAction;
    QMenu *m_rowMenu;
    QMenu *m_contextMenu;
    QMenu *m_headerMenu;
    AddressBookModel * m_model;
    AddressBookProxyModel * m_proxyModel;
    Wallet *m_wallet;
};

#endif // CONTACTSWIDGET_H
