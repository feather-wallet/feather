// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QtCore>
#include <QObject>
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

void UtilsNetworking::get(const QString &url) {
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", m_userAgent.toUtf8());

    QNetworkReply *reply;
    reply = this->m_networkAccessManager->get(request);
    connect(reply, &QNetworkReply::finished, std::bind(&UtilsNetworking::webResponse, this, reply));
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

void UtilsNetworking::webResponse(QNetworkReply *reply) {
    QByteArray data = reply->readAll();
    QString err;
    if (reply->error()) {
        err = reply->errorString();
        qCritical() << err;
        qCritical() << data;
        if (!data.isEmpty())
            err += QString("%1 %2").arg(err).arg(Utils::barrayToString(data));
    }
    reply->deleteLater();

    if(!err.isEmpty())
        emit webErrorReceived(err);
    else
        emit webReceived(data);
}

QString UtilsNetworking::validateJSON(QNetworkReply *reply){
    QList<QByteArray> headerList = reply->rawHeaderList();
    QByteArray headerJson = reply->rawHeader("Content-Type");
    if(headerJson.length() <= 15)
        return "Bad Content-Type";
    QString headerJsonStr = QTextCodec::codecForMib(106)->toUnicode(headerJson);
    int _contentType = headerList.indexOf("Content-Type");
    if (_contentType < 0 || !headerJsonStr.startsWith("application/json"))
        return "Bad Content-Type";
    QByteArray data = reply->readAll();
    if(!Utils::validateJSON(data))
        return "Bad or empty JSON";
    return "OK";
}
