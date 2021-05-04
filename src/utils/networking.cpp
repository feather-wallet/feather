// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "utils/utils.h"
#include "utils/networking.h"

UtilsNetworking::UtilsNetworking(QNetworkAccessManager *networkAccessManager, QObject *parent) :
        QObject(parent),
        m_networkAccessManager(networkAccessManager) {}

void UtilsNetworking::setUserAgent(const QString &userAgent) {
    this->m_userAgent = userAgent;
}

QNetworkReply* UtilsNetworking::get(const QString &url) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());

    return this->m_networkAccessManager->get(request);
}

QNetworkReply* UtilsNetworking::getJson(const QString &url) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    return this->m_networkAccessManager->get(request);
}

QNetworkReply* UtilsNetworking::postJson(const QString &url, const QJsonObject &data) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    QJsonDocument doc(data);
    QByteArray bytes = doc.toJson();
    return this->m_networkAccessManager->post(request, bytes);
}
