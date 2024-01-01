// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "utils/Utils.h"
#include "utils/Networking.h"
#include "utils/NetworkManager.h"
#include "config.h"

Networking::Networking(QObject *parent)
    : QObject(parent) {}

void Networking::setUserAgent(const QString &userAgent) {
    this->m_userAgent = userAgent;
}

QNetworkReply* Networking::get(QObject *parent, const QString &url) {
    if (conf()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());

    QNetworkReply *reply = this->m_networkAccessManager->get(request);;
    reply->setParent(parent);
    return reply;
}

QNetworkReply* Networking::getJson(QObject *parent, const QString &url) {
    if (conf()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    QNetworkReply *reply = this->m_networkAccessManager->get(request);
    reply->setParent(parent);
    return reply;
}

QNetworkReply* Networking::postJson(QObject *parent, const QString &url, const QJsonObject &data) {
    if (conf()->get(Config::offlineMode).toBool()) {
        return nullptr;
    }

    m_networkAccessManager = getNetwork(url);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());
    request.setRawHeader("Content-Type", "application/json");

    QJsonDocument doc(data);
    QByteArray bytes = doc.toJson();

    QNetworkReply *reply = this->m_networkAccessManager->post(request, bytes);
    reply->setParent(parent);
    return reply;
}
