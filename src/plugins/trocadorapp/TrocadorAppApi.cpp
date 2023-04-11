// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TrocadorAppApi.h"

#include "utils/config.h"

TrocadorAppApi::TrocadorAppApi(QObject *parent, Networking *network)
    : QObject(parent)
    , m_network(network)
{
}

void TrocadorAppApi::requestStandard(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                                     const QString &networkTo, const QString &amountFrom, const QString &paymentMethod)
{
    QString url = this->getStandardPaymentUrl(currencyCode, networkFrom, tradeForCode, networkTo, amountFrom, paymentMethod);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::REQUEST_STANDARD));
}

void TrocadorAppApi::requestPayment(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                                     const QString &networkTo, const QString &amountTo, const QString &paymentMethod)
{
    QString url = this->getStandardPaymentUrl(currencyCode, networkFrom, tradeForCode, networkTo, amountTo, paymentMethod);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&TrocadorAppApi::onResponse, this, reply, Endpoint::REQUEST_PAYMENT));
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
        QString errorStr = "Invalid response from TrocadorApp. URL: " + reply->request().url().toString();
        emit ApiResponse(TrocadorAppResponse{false, endpoint, errorStr, {}});
        return;
    }

    if (obj.contains("error")) {
        QString errorStr = QJsonDocument(obj["error"].toObject()).toJson(QJsonDocument::Compact);
        emit ApiResponse(TrocadorAppResponse{false, endpoint, errorStr, obj});
        return;
    }

    emit ApiResponse(TrocadorAppResponse{true, endpoint, "", obj});
}

QString TrocadorAppApi::getStandardPaymentUrl(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                                  const QString &networkTo, const QString &amount, const QString &paymentMethod)
{
    QString url = QString("%1new_rate/").arg(this->getBaseUrl());

    QUrlQuery query;
    if (!amount.isEmpty() && amount != "0"){
        query.addQueryItem("ticker_from", currencyCode.toLower());
        query.addQueryItem("network_from", networkFrom);
        query.addQueryItem("ticker_to", tradeForCode.toLower());
        query.addQueryItem("network_to", networkTo);

        if(paymentMethod == "False"){
            query.addQueryItem("amount_from", amount);
            query.addQueryItem("payment", paymentMethod);
        } else if (paymentMethod == "True"){
            query.addQueryItem("amount_to", amount);
            query.addQueryItem("payment", paymentMethod);
        }
    }
    
    url += "?" + query.toString();
    return url;
}

QString TrocadorAppApi::getBaseUrl() {
    if (config()->get(Config::proxy).toInt() == Config::Proxy::Tor && config()->get(Config::torOnlyAllowOnion).toBool()) {
        return "http://trocadorfyhlu27aefre5u7zri66gudtzdyelymftvr4yjwcxhfaqsid.onion/en/api/";
    }

    if (config()->get(Config::proxy).toInt() == Config::Proxy::i2p) {
        return "http://trocador.i2p/en/api/";
    }

    return "https://trocador.app/en/api/";
}