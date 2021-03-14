// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_RECEIVEWIDGET_H
#define FEATHER_RECEIVEWIDGET_H

#include "appcontext.h"
#include "qrcode/QrCode.h"
#include "libwalletqt/Subaddress.h"
#include "model/SubaddressProxyModel.h"
#include "model/SubaddressModel.h"

#include <QWidget>
#include <QtSvg/QSvgWidget>

namespace Ui {
    class ReceiveWidget;
}

class ReceiveWidget : public QWidget
{
Q_OBJECT

public:
    explicit ReceiveWidget(QWidget *parent = nullptr);
    void setModel(SubaddressModel * model, Wallet * wallet);
    ~ReceiveWidget() override;


public slots:
    void copyAddress();
    void copyLabel();
    void editLabel();
    void showContextMenu(const QPoint& point);
    void setShowFullAddresses(bool show);
    void setShowUsedAddresses(bool show);
    void setShowHiddenAddresses(bool show);
    void setSearchFilter(const QString &filter);
    void onShowTransactions();
    void resetModel();

signals:
    void generateSubaddress();
    void showTransactions(const QString& address);

private slots:
    void showHeaderMenu(const QPoint& position);
    void hideAddress();
    void showAddress();

private:
    Ui::ReceiveWidget *ui;
    QMenu *m_headerMenu;
    QAction *m_showFullAddressesAction;
    QAction *m_showUsedAddressesAction;
    QAction *m_showTransactionsAction;
    Subaddress * m_subaddress;
    SubaddressModel * m_model;
    SubaddressProxyModel * m_proxyModel;
    Wallet * m_wallet;

    void updateQrCode();
    void showQrCodeDialog();
    QStringList getHiddenAddresses();
    void addHiddenAddress(const QString& address);
    void removeHiddenAddress(const QString& address);
    Monero::SubaddressRow* currentEntry();
};

#endif //FEATHER_RECEIVEWIDGET_H
