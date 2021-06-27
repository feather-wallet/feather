// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CONTACTSWIDGET_H
#define FEATHER_CONTACTSWIDGET_H

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
    explicit ContactsWidget(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
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
    void fillAddress(QString &address);

private slots:
    void showHeaderMenu(const QPoint &position);

private:
    QScopedPointer<Ui::ContactsWidget> ui;
    QSharedPointer<AppContext> m_ctx;

    QAction *m_showFullAddressesAction;
    QMenu *m_rowMenu;
    QMenu *m_contextMenu;
    QMenu *m_headerMenu;
    AddressBookModel * m_model;
    AddressBookProxyModel * m_proxyModel;
};

#endif // FEATHER_CONTACTSWIDGET_H
