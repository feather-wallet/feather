// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_CONTACTSWIDGET_H
#define FEATHER_CONTACTSWIDGET_H

#include <QPushButton>
#include <QWidget>
#include <QMenu>

class AddressBookModel;
class AddressBookProxyModel;
class Wallet;

namespace Ui {
class ContactsWidget;
}

class ContactsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactsWidget(Wallet *wallet, QWidget *parent = nullptr);
    ~ContactsWidget() override;

    void setSearchbarVisible(bool visible);
    void focusSearchbar();

public slots:
    void copyAddress();
    void copyName();
    void payTo();
    void newContact(QString address = "", QString name = "");
    void deleteContact();
    void setShowFullAddresses(bool show);
    void setSearchFilter(const QString &filter);

signals:
    void fill(QString &address, QString &description);

private slots:
    void showHeaderMenu(const QPoint &position);

private:
    QScopedPointer<Ui::ContactsWidget> ui;
    Wallet *m_wallet;

    QAction *m_showFullAddressesAction;
    QMenu *m_rowMenu;
    QMenu *m_contextMenu;
    QMenu *m_headerMenu;
    AddressBookModel * m_model;
    AddressBookProxyModel * m_proxyModel;
    QPushButton *m_btn_addContact;
};

#endif // FEATHER_CONTACTSWIDGET_H
