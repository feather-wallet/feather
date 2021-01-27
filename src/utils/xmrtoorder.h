// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_XMRTOORDER_H
#define FEATHER_XMRTOORDER_H

#include <QObject>

#include "utils/networking.h"
#include "PendingTransaction.h"
#include "utils/xmrtoapi.h"

enum OrderState {
    Status_Idle,
    Status_OrderCreating,
    Status_OrderToBeCreated,
    Status_OrderUnpaid,
    Status_OrderXMRSent,
    Status_OrderUnderPaid,
    Status_OrderPaidUnconfirmed,
    Status_OrderPaid,
    Status_OrderBTCSent,
    Status_OrderTimedOut,
    Status_OrderFailed
};

struct XmrToRates {
    double price;
    double upper_limit;
    double lower_limit;
    double ln_upper_limit;
    double ln_lower_limit;
    double zero_conf_max_amount;
    bool zero_conf_enabled;
};

class XmrToOrder : public QObject {
    Q_OBJECT

public:
    explicit XmrToOrder(AppContext *ctx, UtilsNetworking *network, QString baseUrl, bool clearnet, XmrToRates *rates, QObject *parent = nullptr);
    void create(double btcAmount, const QString &currency, const QString &btcAddress);
    void changeState(OrderState state);
    void changeState(const QString &state);
    void stop();

    int state;
    int countdown = -1; // seconds remaining calculated from `seconds_till_timeout`
    QString uuid;
    QString errorMsg;
    QString errorCode;

    double btc_amount = 0;
    QString btc_dest_address;
    QString btc_txid;
    QString xmr_txid;
    bool uses_lightning = false;

    QString receiving_subaddress;
    QString created_at;
    QString expires_at;
    int seconds_till_timeout = -1;
    double incoming_amount_total = 0;       // amount_in_incoming_currency_for_this_order_as_string
    double remaining_amount_incoming;   // amount_in_incoming_currency_that_the_user_must_still_send_as_string
    double incoming_price_btc = 0;          // price_of_1_incoming_in_btc_currency_as_offered_by_service

public slots:
    void onCountdown();
    void onTransactionCancelled(const QVector<QString> &address, double amount);
    void onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);

    void onCreatedError();
    void onChecked(const QJsonObject &object);
    void onCheckedError(const XmrToError& err);
    void check();

private:
    bool m_created = false;
    QString m_baseUrl;
    QTimer m_checkTimer;
    QTimer m_countdownTimer;
    int m_checkFailures = 0;
    bool m_clearnet;
    bool m_paymentSent = false;
    bool m_paymentRequested = false;
    UtilsNetworking *m_network;
    AppContext *m_ctx;
    XmrToRates *m_rates;
    XmrToApi *m_api;

    ~XmrToOrder();

signals:
    void orderChanged();
    void orderPaymentRequired(XmrToOrder *order);
    void orderPaid(XmrToOrder *order);
    void orderPaidUnconfirmed(XmrToOrder *order);
    void orderFailed(XmrToOrder *order);

private slots:
    void onCreated(const QJsonObject &object);
    void onApiFailure(const XmrToResponse &resp);
    void onApiResponse(const XmrToResponse &resp);
};

#endif //FEATHER_XMRTOORDER_H
