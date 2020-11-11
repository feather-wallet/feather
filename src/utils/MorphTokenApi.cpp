// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "MorphTokenApi.h"

MorphTokenApi::MorphTokenApi(QObject *parent, UtilsNetworking *network, QString baseUrl)
        : QObject(parent)
        , m_network(network)
        , m_baseUrl(std::move(baseUrl))
{
}

void MorphTokenApi::createTrade(const QString &inputAsset, const QString &outputAsset, const QString &refundAddress, const QString &outputAddress) {
    QJsonObject trade;

    QJsonObject input;
    input["asset"] = inputAsset;
    input["refund"] = refundAddress;

    QJsonArray output;
    QJsonObject outputObj;
    outputObj["asset"] = outputAsset;
    outputObj["weight"] = 10000;
    outputObj["address"] = outputAddress;
    output.append(outputObj);

    trade["input"] = input;
    trade["output"] = output;

    QString url = QString("%1/morph").arg(m_baseUrl);
    QNetworkReply *reply = m_network->postJson(url, trade);
    connect(reply, &QNetworkReply::finished, std::bind(&MorphTokenApi::onResponse, this, reply, Endpoint::CREATE_TRADE));
}

void MorphTokenApi::getTrade(const QString &morphId) {
    QString url = QString("%1/morph/%2").arg(m_baseUrl, morphId);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&MorphTokenApi::onResponse, this, reply, Endpoint::GET_TRADE));
}

void MorphTokenApi::getRates() {
    QString url = QString("%1/rates").arg(m_baseUrl);
    QNetworkReply *reply = m_network->getJson(url);
    connect(reply, &QNetworkReply::finished, std::bind(&MorphTokenApi::onResponse, this, reply, Endpoint::GET_RATES));
}

void MorphTokenApi::getLimits(const QString &inputAsset, const QString &outputAsset) {
    QJsonObject limits;

    QJsonObject input;
    input["asset"] = inputAsset;

    QJsonArray output;
    QJsonObject outputObj;
    outputObj["asset"] = outputAsset;
    outputObj["weight"] = 10000;
    output.append(outputObj);

    limits["input"] = input;
    limits["output"] = output;

    QString url = QString("%1/limits").arg(m_baseUrl);
    QNetworkReply *reply = m_network->postJson(url, limits);
    connect(reply, &QNetworkReply::finished, std::bind(&MorphTokenApi::onResponse, this, reply, Endpoint::GET_LIMITS));
}

void MorphTokenApi::onResponse(QNetworkReply *reply, Endpoint endpoint) {
    const auto ok = reply->error() == QNetworkReply::NoError;
    const auto err = reply->errorString();

    QByteArray data = reply->readAll();
    QJsonObject obj;
    if (!data.isEmpty() && Utils::validateJSON(data)) {
        auto doc = QJsonDocument::fromJson(data);
        obj = doc.object();
    }
    else if (!ok) {
        emit ApiResponse(MorphTokenResponse(false, endpoint, err, {}));
        return;
    }
    else {
        emit ApiResponse(MorphTokenResponse(false, endpoint, "Invalid response from MorphToken", {}));
        return;
    }

    if (obj.contains("success")) {
        emit ApiResponse(MorphTokenResponse(false, endpoint, obj.value("description").toString(), obj));
        return;
    }

    reply->deleteLater();
    emit ApiResponse(MorphTokenResponse(true, endpoint, "", obj));
}