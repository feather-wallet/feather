// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "LocalMoneroApi.h"

LocalMoneroApi::LocalMoneroApi(QObject *parent, UtilsNetworking *network, const QString &baseUrl)
    : QObject(parent)
    , m_network(network)
    , m_baseUrl(baseUrl)
{
}

void LocalMoneroApi::countryCodes() {
    QString url = QString("%1/countrycodes").arg(m_baseUrl);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::COUNTRY_CODES));
}

void LocalMoneroApi::currencies() {
    QString url = QString("%1/currencies").arg(m_baseUrl);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::CURRENCIES));
}

void LocalMoneroApi::paymentMethods(const QString &countryCode) {
    QString url;
    if (countryCode.isEmpty()) {
        url = QString("%1/payment_methods").arg(m_baseUrl);
    } else {
        url = QString("%1/payment_methods/%2").arg(m_baseUrl, countryCode);
    }
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::PAYMENT_METHODS));
}

void LocalMoneroApi::buyMoneroOnline(const QString &currencyCode, const QString &countryCode,
                                     const QString &paymentMethod, const QString &amount, int page)
{
    QString url = this->getBuySellUrl(true, currencyCode, countryCode, paymentMethod, amount, page);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::BUY_MONERO_ONLINE));
}

void LocalMoneroApi::sellMoneroOnline(const QString &currencyCode, const QString &countryCode,
                                      const QString &paymentMethod, const QString &amount, int page)
{
    QString url = this->getBuySellUrl(false, currencyCode, countryCode, paymentMethod, amount, page);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::SELL_MONERO_ONLINE));
}

void LocalMoneroApi::accountInfo(const QString &username) {
    QString url = QString("%1/account_info/%2").arg(m_baseUrl, username);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&LocalMoneroApi::onResponse, this, reply, Endpoint::ACCOUNT_INFO));
}

void LocalMoneroApi::onResponse(QNetworkReply *reply, LocalMoneroApi::Endpoint endpoint) {
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
        emit ApiResponse(LocalMoneroResponse{false, endpoint, err, {}});
        return;
    }
    else {
        emit ApiResponse(LocalMoneroResponse{false, endpoint, "Invalid response from LocalMonero", {}});
        return;
    }

    if (obj.contains("error")) {
        QString errorStr = QJsonDocument(obj["error"].toObject()).toJson(QJsonDocument::Compact);
        emit ApiResponse(LocalMoneroResponse{false, endpoint, errorStr, obj});
        return;
    }

    emit ApiResponse(LocalMoneroResponse{true, endpoint, "", obj});
}

QString LocalMoneroApi::getBuySellUrl(bool buy, const QString &currencyCode, const QString &countryCode,
                                      const QString &paymentMethod, const QString &amount, int page)
{
    QString url = QString("%1/%2-monero-online/%3").arg(m_baseUrl, buy ? "buy" : "sell", currencyCode);
    if (!countryCode.isEmpty() && paymentMethod.isEmpty())
        url += QString("/%1").arg(countryCode);
    else if (countryCode.isEmpty() && !paymentMethod.isEmpty())
        url += QString("/%1").arg(paymentMethod);
    else if (!countryCode.isEmpty() && !paymentMethod.isEmpty())
        url += QString("/%1/%2").arg(countryCode, paymentMethod);

    QUrlQuery query;
    if (!amount.isEmpty() && amount != "0")
        query.addQueryItem("amount", amount);
    if (page > 0)
        query.addQueryItem("page", QString::number(page));
    url += "?" + query.toString();
    return url;
}