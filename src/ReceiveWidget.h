// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_RECEIVEWIDGET_H
#define FEATHER_RECEIVEWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QSvgWidget>

#include "libwalletqt/Subaddress.h"
#include "libwalletqt/Wallet.h"
#include "model/SubaddressProxyModel.h"
#include "model/SubaddressModel.h"
#include "qrcode/QrCode.h"

namespace Ui {
    class ReceiveWidget;
}

class ReceiveWidget : public QWidget
{
Q_OBJECT

public:
    explicit ReceiveWidget(Wallet *wallet, QWidget *parent = nullptr);
    ~ReceiveWidget() override;

    void setSearchbarVisible(bool visible);
    void focusSearchbar();

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
    void createPaymentRequest();

signals:
    void showTransactions(const QString& address);

private slots:
    void showHeaderMenu(const QPoint& position);
    void hideAddress();
    void showAddress();
    void showOnDevice();
    void generateSubaddress();

private:
    QScopedPointer<Ui::ReceiveWidget> ui;
    Wallet *m_wallet;
    QMenu *m_headerMenu;
    QAction *m_showFullAddressesAction;
    QAction *m_showTransactionsAction;
    SubaddressModel *m_model;
    SubaddressProxyModel *m_proxyModel;

    void updateQrCode();
    void showQrCodeDialog();
    QStringList getHiddenAddresses();
    void addHiddenAddress(const QString& address);
    void removeHiddenAddress(const QString& address);
    Monero::SubaddressRow* currentEntry();
};

#endif //FEATHER_RECEIVEWIDGET_H
