// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_XMRTOCONVERT_H
#define FEATHER_XMRTOCONVERT_H

#include <QObject>

#include "model/XmrToModel.h"
#include "utils/xmrtoorder.h"
#include "utils/xmrtoapi.h"

class AppContext;
class XmrTo: public QObject {
    Q_OBJECT

public:
    explicit XmrTo(AppContext *ctx, QObject *parent = nullptr);
    Q_ENUM(OrderState);

    XmrToModel *tableModel;
    static QMap<OrderState, QString> stateMap;
    XmrToRates rates;
    QList<XmrToOrder*> orders;

public slots:
    void createOrder(double amount, const QString &currency, const QString &btcAddress);
    void onGetRates();
    void onRatesReceived(const QJsonObject &doc);
    void onViewOrder(const QString &orderId);
    void onNetworkChanged(bool clearnet);
    void onWalletOpened();
    void onWalletClosed();

private slots:
    void onApiResponse(const XmrToResponse &doc);

signals:
    void orderPaymentRequired(XmrToOrder *order);
    void orderPaidUnconfirmed(XmrToOrder *order);
    void orderPaid(XmrToOrder *order);
    void orderFailed(XmrToOrder *order);
    void ratesUpdated(XmrToRates rates);
    void openURL(const QString &url);
    void connectionError(const QString &err);
    void connectionSuccess();

private:
    void onApiFailure(const XmrToResponse &doc);

    QString m_baseUrl;
    AppContext *m_ctx;
    int m_orderTimeout = 900;  // https://xmrto-api.readthedocs.io/en/latest/introduction.html#various-parameters

    UtilsNetworking *m_netTor;
    UtilsNetworking *m_netClear;

    XmrToApi *m_api;
    XmrToApi *m_apiTor;
    XmrToApi *m_apiClear;
};

#endif //FEATHER_XMRTOCONVERT_H
