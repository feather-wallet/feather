// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_MORPHTOKENAPI_H
#define FEATHER_MORPHTOKENAPI_H

#include <QObject>
#include <utility>
#include "utils/networking.h"

class MorphTokenApi : public QObject {
    Q_OBJECT

public:
    enum Endpoint {
        CREATE_TRADE = 0,
        GET_TRADE,
        GET_RATES,
        GET_LIMITS
    };

    struct MorphTokenResponse {
        explicit MorphTokenResponse(bool ok, Endpoint endpoint, QString message, QJsonObject obj)
                : ok(ok), endpoint(endpoint), message(std::move(message)), obj(std::move(obj)) {};

        bool ok;
        Endpoint endpoint;
        QString message;
        QJsonObject obj;
    };

    explicit MorphTokenApi(QObject *parent, UtilsNetworking *network, QString baseUrl = "https://api.morphtoken.com");

    void createTrade(const QString &inputAsset, const QString &outputAsset, const QString &refundAddress, const QString &outputAddress);
    void getTrade(const QString &morphId);
    void getRates();
    void getLimits(const QString &inputAsset, const QString &outputAsset);

signals:
    void ApiResponse(MorphTokenResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);

private:
    QString m_baseUrl;
    UtilsNetworking *m_network;
};


#endif //FEATHER_MORPHTOKENAPI_H
