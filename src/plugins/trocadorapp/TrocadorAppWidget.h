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
    void onRadioButtonToggled();

private slots:
    void onSearchClicked();
    void onApiResponse(const TrocadorAppApi::TrocadorAppResponse &resp);

private:
    void searchOffers();
    void showContextMenu(const QPoint &point);
    void openOfferUrl();

    QScopedPointer<Ui::TrocadorAppWidget> ui;
    Wallet *m_wallet;

    TrocadorAppApi *m_api;
    TrocadorAppModel *m_model;
    Networking *m_network;
};


#endif //FEATHER_TROCADORAPPWIDGET_H
