// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_MORPHTOKENWIDGET_H
#define FEATHER_MORPHTOKENWIDGET_H

#include <QWidget>
#include "appcontext.h"
#include "utils/MorphTokenApi.h"

namespace Ui {
    class MorphTokenWidget;
}

class MorphTokenWidget : public QWidget
{
Q_OBJECT

public:
    explicit MorphTokenWidget(QWidget *parent = nullptr);
    ~MorphTokenWidget() override;

private:
    void createTrade();
    void lookupTrade();
    void getRates();
    void onApiResponse(const MorphTokenApi::MorphTokenResponse &resp);

    void onCountdown();
    void displayRate();

    QString formatAmount(const QString &asset, double amount);

    Ui::MorphTokenWidget *ui;

    AppContext *m_ctx;
    MorphTokenApi *m_api;
    UtilsNetworking *m_network;
    QTimer m_countdownTimer;
    int m_countdown = 30;
    QJsonObject m_rates;
    QTimer m_ratesTimer;
};

#endif //FEATHER_MORPHTOKENWIDGET_H
