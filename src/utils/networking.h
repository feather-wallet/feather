// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_NETWORKING_H
#define FEATHER_NETWORKING_H

#include <QtCore>
#include <QRegExp>
#include <QApplication>
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "utils/utils.h"

static QStringList randomHTTPAgents;
class CCSEntry;

class UtilsNetworking : public QObject
{
Q_OBJECT

public:
    explicit UtilsNetworking(QNetworkAccessManager *networkAccessManager, QObject *parent = nullptr);

    void get(const QString &url);
    QNetworkReply* getJson(const QString &url);
    QNetworkReply* postJson(const QString &url, const QJsonObject &data);
    void setUserAgent(const QString &userAgent);
    static QString validateJSON(QNetworkReply *reply);

private slots:
    void webResponse(QNetworkReply *reply);

signals:
    void webErrorReceived(QString msg);
    void webReceived(QByteArray data);

private:
    QString m_userAgent = "Mozilla/5.0 (Windows NT 10.0; rv:68.0) Gecko/20100101 Firefox/68.0";
    QNetworkAccessManager *m_networkAccessManager;
};

#endif //FEATHER_NETWORKING_H
