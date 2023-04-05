// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TrocadorAppApi.h"

#include "utils/config.h"

TrocadorAppApi::TrocadorAppApi(QObject *parent, Networking *network)
    : QObject(parent)
    , m_network(network)
{
}

void TrocadorAppApi::currencies() {
    QString url = QString("%1/currencies").arg(this->getBaseUrl());
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::CURRENCIES));
}

void TrocadorAppApi::paymentMethods() {
    QString url;
    url = QString("%1/spread").arg(this->getBaseUrl());
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::SPREAD));
}

void TrocadorAppApi::buyMoneroOnline(const QString &currencyCode,
                                     const QString &paymentMethod, const QString &amount, int page)
{
    QString url = this->getBuySellUrl(true, currencyCode, paymentMethod, amount, page);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::BUY_MONERO_ONLINE));
}

void TrocadorAppApi::sellMoneroOnline(const QString &currencyCode,
                                      const QString &paymentMethod, const QString &amount, int page)
{
    QString url = this->getBuySellUrl(false, currencyCode, paymentMethod, amount, page);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::SELL_MONERO_ONLINE));
}

void TrocadorAppApi::accountInfo(const QString &username) {
    QString url = QString("%1/account_info/%2").arg(this->getBaseUrl(), username);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::ACCOUNT_INFO));
}

void TrocadorAppApi::onResponse(QNetworkReply *reply, TrocadorAppApi::Endpoint endpoint) {
    const bool ok = reply->error() == QNetworkReply::NoError;
    const QString err = reply->errorString();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonObject obj;
    if (!data.isEmpty() && Utils::validateJSON(data)) {
        auto doc = QJsonDocument::fromJson(data);
        obj = doc.object();
    }
    else if (!ok) {
        emit ApiResponse(TrocadorAppResponse{false, endpoint, err, {}});
        return;
    }
    else {
        emit ApiResponse(TrocadorAppResponse{false, endpoint, "Invalid response from TrocadorApp", {}});
        return;
    }

    if (obj.contains("error")) {
        QString errorStr = QJsonDocument(obj["error"].toObject()).toJson(QJsonDocument::Compact);
        emit ApiResponse(TrocadorAppResponse{false, endpoint, errorStr, obj});
        return;
    }

    emit ApiResponse(TrocadorAppResponse{true, endpoint, "", obj});
}

QString TrocadorAppApi::getBuySellUrl(bool buy, const QString &currencyCode,
                                      const QString &paymentMethod, const QString &amount, int page)
{
    QString url = QString("%1/%2-monero-online/%3").arg(this->getBaseUrl(), buy ? "buy" : "sell", currencyCode);
    if (!paymentMethod.isEmpty())
        url += QString("/%1").arg(paymentMethod);

    QUrlQuery query;
    if (!amount.isEmpty() && amount != "0")
        query.addQueryItem("amount", amount);
    if (page > 0)
        query.addQueryItem("page", QString::number(page));
    url += "?" + query.toString();
    return url;
}

QString TrocadorAppApi::getBaseUrl() {
    if (config()->get(Config::proxy).toInt() == Config::Proxy::Tor && config()->get(Config::torOnlyAllowOnion).toBool()) {
        return "http://nehdddktmhvqklsnkjqcbpmb63htee2iznpcbs5tgzctipxykpj6yrid.onion/api/v1";
    }

    if (config()->get(Config::proxy).toInt() == Config::Proxy::i2p) {
        return "http://yeyar743vuwmm6fpgf3x6bzmj7fxb5uxhuoxx4ea76wqssdi4f3q.b32.i2p/api/v1";
    }

    return "https://agoradesk.com/api/v1";
}