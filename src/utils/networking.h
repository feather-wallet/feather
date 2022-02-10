// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_NETWORKING_H
#define FEATHER_NETWORKING_H

#include <QRegExp>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "utils/Utils.h"

class UtilsNetworking : public QObject
{
Q_OBJECT

public:
    explicit UtilsNetworking(QNetworkAccessManager *networkAccessManager, QObject *parent = nullptr);

    QNetworkReply* get(const QString &url);
    QNetworkReply* getJson(const QString &url);
    QNetworkReply* postJson(const QString &url, const QJsonObject &data);
    void setUserAgent(const QString &userAgent);

private:
    QString m_userAgent = "Mozilla/5.0 (Windows NT 10.0; rv:68.0) Gecko/20100101 Firefox/68.0";
    QNetworkAccessManager *m_networkAccessManager;
};

#endif //FEATHER_NETWORKING_H
