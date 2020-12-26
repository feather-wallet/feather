// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "xmrto.h"

#include "utils/xmrtoorder.h"
#include "appcontext.h"

QMap<OrderState, QString> XmrTo::stateMap;

XmrTo::XmrTo(AppContext *ctx, QObject *parent) :
        QObject(parent),
        m_ctx(ctx) {

    m_baseUrl = m_ctx->networkType == NetworkType::Type::MAINNET ? "https://xmr.to" : "https://test.xmr.to";

    m_netTor = new UtilsNetworking(this->m_ctx->network);
    m_netClear = new UtilsNetworking(this->m_ctx->networkClearnet);

    m_apiTor = new XmrToApi(this, m_netTor, m_baseUrl);
    m_apiClear = new XmrToApi(this, m_netClear, m_baseUrl);

    connect(m_apiTor, &XmrToApi::ApiResponse, this, &XmrTo::onApiResponse);
    connect(m_apiClear, &XmrToApi::ApiResponse, this, &XmrTo::onApiResponse);

    connect(this, &XmrTo::orderPaymentRequired, this->m_ctx, QOverload<XmrToOrder*>::of(&AppContext::onCreateTransaction));

    XmrTo::stateMap[OrderState::Status_Idle] = "IDLE";
    XmrTo::stateMap[OrderState::Status_OrderCreating] = "CREATING";
    XmrTo::stateMap[OrderState::Status_OrderUnpaid] = "UNPAID";
    XmrTo::stateMap[OrderState::Status_OrderToBeCreated] = "TO_BE_CREATED";
    XmrTo::stateMap[OrderState::Status_OrderUnderPaid] = "UNDERPAID";
    XmrTo::stateMap[OrderState::Status_OrderPaidUnconfirmed] = "PAID_UNCONFIRMED";
    XmrTo::stateMap[OrderState::Status_OrderPaid] = "PAID";
    XmrTo::stateMap[OrderState::Status_OrderBTCSent] = "BTC_SENT";
    XmrTo::stateMap[OrderState::Status_OrderTimedOut] = "TIMED_OUT";
    XmrTo::stateMap[OrderState::Status_OrderFailed] = "FAILED";
    XmrTo::stateMap[OrderState::Status_OrderXMRSent] = "XMR_SENT";

    this->tableModel = new XmrToModel(&this->orders, this);
}

void XmrTo::createOrder(double amount, const QString &currency, const QString &btcAddress) {
    // ^[13][a-km-zA-HJ-NP-Z0-9]{26,33}$

    XmrToOrder *order;
    order = new XmrToOrder(this->m_ctx, m_netTor, m_baseUrl, false, &this->rates, this);

    connect(order, &XmrToOrder::orderFailed, this, &XmrTo::orderFailed);
    connect(order, &XmrToOrder::orderPaid, this, &XmrTo::orderPaid);
    connect(order, &XmrToOrder::orderPaidUnconfirmed, this, &XmrTo::orderPaidUnconfirmed);
    connect(order, &XmrToOrder::orderPaymentRequired, this, &XmrTo::orderPaymentRequired);
    connect(order, &XmrToOrder::orderChanged, this->tableModel, &XmrToModel::update);

    order->create(amount, currency, btcAddress);
    this->orders.append(order);
    tableModel->update();
}


void XmrTo::onApiResponse(const XmrToResponse &resp) {
    if (!resp.ok) {
        this->onApiFailure(resp);
        return;
    }

    emit connectionSuccess();
    if (resp.endpoint == Endpoint::RATES) {
        onRatesReceived(resp.obj);
    }
}

void XmrTo::onApiFailure(const XmrToResponse &resp) {
    emit connectionError(resp.message);
}

void XmrTo::onGetRates() {
    m_apiTor->getRates();
}

void XmrTo::onRatesReceived(const QJsonObject &doc) {
    this->rates.price = doc.value("price").toString().toDouble();
    this->rates.ln_lower_limit = doc.value("ln_lower_limit").toString().toDouble();
    this->rates.ln_upper_limit = doc.value("ln_upper_limit").toString().toDouble();
    this->rates.lower_limit = doc.value("lower_limit").toString().toDouble();
    this->rates.upper_limit = doc.value("upper_limit").toString().toDouble();
    this->rates.zero_conf_enabled = doc.value("zero_conf_enabled").toBool();
    this->rates.zero_conf_max_amount = doc.value("zero_conf_enabled").toString().toDouble();
    emit ratesUpdated(rates);
}

void XmrTo::onNetworkChanged(bool clearnet) {
    m_api = clearnet ? m_apiClear : m_apiTor;
}

void XmrTo::onWalletClosed() {
    // @TODO: cleanup
    for(const auto &order: this->orders)
        order->deleteLater();

    this->tableModel->update();
}

void XmrTo::onWalletOpened() {
    // @TODO: read past XMR.To orders, start pending ones
}

void XmrTo::onViewOrder(const QString &orderId) {
    QString url = QString("%1/nojs/status/%2").arg(this->m_baseUrl).arg(orderId);
    emit openURL(url);
}
