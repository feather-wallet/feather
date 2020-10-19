// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "xmrtoapi.h"

#include <utility>

XmrToApi::XmrToApi(QObject *parent, UtilsNetworking *network, QString baseUrl)
    : QObject(parent)
    , m_network(network)
    , m_baseUrl(std::move(baseUrl))
{
}

void XmrToApi::getRates() {
    QString url = QString("%1/api/v3/xmr2btc/order_parameter_query/").arg(this->m_baseUrl);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&XmrToApi::onResponse, this, reply, Endpoint::RATES));
}

void XmrToApi::createOrder(double amount, const QString &amount_currency, const QString &dest_address) {
    QJsonObject order;
    order["amount"] = amount;
    order["amount_currency"] = amount_currency;
    order["btc_dest_address"] = dest_address;

    QString url = QString("%1/api/v3/xmr2btc/order_create/").arg(m_baseUrl);
    QNetworkReply *reply = m_network->postJson(url, order);
    connect(reply, &QNetworkReply::finished, std::bind(&XmrToApi::onResponse, this, reply, Endpoint::ORDER_CREATE));
}

void XmrToApi::getOrderStatus(const QString &uuid) {
    QJsonObject order;
    order["uuid"] = uuid;

    QString url = QString("%1/api/v3/xmr2btc/order_status_query/").arg(m_baseUrl);
    QNetworkReply *reply = m_network->postJson(url, order);
    connect(reply, &QNetworkReply::finished, std::bind(&XmrToApi::onResponse, this, reply, Endpoint::ORDER_STATUS));
}

void XmrToApi::onResponse(QNetworkReply *reply, Endpoint endpoint) {
    const auto ok = reply->error() == QNetworkReply::NoError;
    const auto err = reply->errorString();

    QByteArray data = reply->readAll();
    QJsonObject obj;
    if (!data.isEmpty() && Utils::validateJSON(data)) {
        auto doc = QJsonDocument::fromJson(data);
        obj = doc.object();
    }
    else if (!ok) {
        emit ApiResponse(XmrToResponse(false, endpoint, err));
        return;
    }
    else {
        emit ApiResponse(XmrToResponse(false, endpoint, "Invalid response from XMR.to"));
        return;
    }

    XmrToError xmrto_err = XmrToApi::getApiError(obj);
    if (!xmrto_err.code.isEmpty()) {
        emit ApiResponse(XmrToResponse(false, endpoint, m_errorMap.contains(xmrto_err.code) ? m_errorMap[xmrto_err.code] : "", xmrto_err));
        return;
    }

    reply->deleteLater();
    emit ApiResponse(XmrToResponse(true, endpoint, "", obj));
}

XmrToError XmrToApi::getApiError(const QJsonObject &obj) {
    if (!obj.contains("error"))
        return XmrToError();

    QString code = obj.value("error").toString();
    QString msg = obj.value("error_msg").toString();

    return XmrToError(code, msg);
}
