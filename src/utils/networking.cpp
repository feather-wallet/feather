// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "utils/Utils.h"
#include "utils/networking.h"
#include "utils/NetworkManager.h"
#include "config.h"

UtilsNetworking::UtilsNetworking(QObject *parent)
    : QObject(parent) {}

void UtilsNetworking::setUserAgent(const QString &userAgent) {
    this->m_userAgent = userAgent;
}

QNetworkReply* UtilsNetworking::get(const QString &url) {
    if (config()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());

    return this->m_networkAccessManager->get(request);
}

QNetworkReply* UtilsNetworking::getJson(const QString &url) {
    if (config()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    return this->m_networkAccessManager->get(request);
}

QNetworkReply* UtilsNetworking::postJson(const QString &url, const QJsonObject &data) {
    if (config()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    QJsonDocument doc(data);
    QByteArray bytes = doc.toJson();
    return this->m_networkAccessManager->post(request, bytes);
}
