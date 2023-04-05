// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TROCADORAPPWIDGET_H
#define FEATHER_TROCADORAPPWIDGET_H

#include <QWidget>

#include "TrocadorAppApi.h"
#include "TrocadorAppModel.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class TrocadorAppWidget;
}

class TrocadorAppWidget : public QWidget
{
Q_OBJECT

public:
    explicit TrocadorAppWidget(QWidget *parent, Wallet *wallet);
    ~TrocadorAppWidget() override;

public slots:
    void skinChanged();
    void onRadioButtonToggled();

private slots:
    void onSearchClicked();
    void onApiResponse(const TrocadorAppApi::TrocadorAppResponse &resp);
    void onLoadMore();
    void onWsCurrenciesReceived(const QJsonArray &currencies);
    void onWsPaymentMethodsReceived(const QJsonObject &payment_methods);

private:
    void searchOffers(int page = 0);
    void showContextMenu(const QPoint &point);
    void openOfferUrl();
    void viewOfferDetails();

    QScopedPointer<Ui::TrocadorAppWidget> ui;
    Wallet *m_wallet;

    int m_currentPage = 0;

    TrocadorAppApi *m_api;
    TrocadorAppModel *m_model;
    Networking *m_network;
    QJsonObject m_paymentMethods;
};


#endif //FEATHER_TROCADORAPPWIDGET_H
