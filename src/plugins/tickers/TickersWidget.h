// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef TICKERSWIDGET_H
#define TICKERSWIDGET_H

#include <QWidget>

#include "Wallet.h"
#include "widgets/TickerWidget.h"

namespace Ui {
    class TickersWidget;
}

class TickersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TickersWidget(QWidget *parent, Wallet *wallet);
    ~TickersWidget() override;

private slots:
    void updateBalance();
    void updateDisplay();

private:
    void setup();

    QScopedPointer<Ui::TickersWidget> ui;
    Wallet *m_wallet;

    QList<TickerWidgetBase*> m_tickerWidgets;
    QScopedPointer<BalanceTickerWidget> m_balanceTickerWidget;
};

#endif //TICKERSWIDGET_H
