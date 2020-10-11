// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef XMRTOWIDGET_H
#define XMRTOWIDGET_H

#include <QWidget>
#include "widgets/tickerwidget.h"
#include "utils/xmrto.h"
#include "appcontext.h"

namespace Ui {
    class XMRToWidget;
}

class XMRToWidget : public QWidget
{
Q_OBJECT

public:
    explicit XMRToWidget(QWidget *parent = nullptr);
    void setHistoryModel(XmrToModel *model);
    ~XMRToWidget();

public slots:
    void onWalletClosed();
    void onGetRates();
    void onConnectionError(QString msg);
    void onConnectionSuccess();
    void onRatesUpdated(XmrToRates rates);
    void onTorCheckBoxToggled(int state);
    void onCreateOrder();
    void onBalanceUpdated(double balance, double unlocked, const QString &balance_str, const QString &unlocked_str);
    void updateConversionLabel();

    void onInitiateTransaction();
    void onEndTransaction();

signals:
    void getRates();
    void networkChanged(bool clearnet);
    void createOrder(double btnAmount, QString currency, QString btnAddress);
    void viewOrder(const QString &orderId);

private:
    void showInfoDialog();

    QMap<QString, TickerWidget*> m_tickerWidgets;
    QMenu *m_contextMenu;
    QAction *m_viewOnXmrToAction;
    QAction *m_showDetailsAction;

    Ui::XMRToWidget *ui;
    AppContext *m_ctx;
    bool m_ratesDisplayed = false;
    const QString m_regionBlockMessage = "Beware that XMR.To region blocks certain IPs, which can be problematic in combination with Tor. "
                                         "Wait a few minutes for the circuit to switch, or disable the option to relay over Tor if the problem persists.";
    double m_unlockedBalance = 0;
    XmrToModel *tableModel;

    enum curr {
        BTC = 0,
        XMR
    };
};

#endif
