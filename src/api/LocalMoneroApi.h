// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_LOCALMONEROAPI_H
#define FEATHER_LOCALMONEROAPI_H

#include <QObject>
#include "utils/networking.h"

class LocalMoneroApi : public QObject {
    Q_OBJECT

public:
    enum Endpoint {
        COUNTRY_CODES,
        CURRENCIES,
        PAYMENT_METHODS,
        BUY_MONERO_ONLINE,
        SELL_MONERO_ONLINE,
        ACCOUNT_INFO
    };

    struct LocalMoneroResponse {
        bool ok;
        Endpoint endpoint;
        QString message;
        QJsonObject obj;
    };

    explicit LocalMoneroApi(QObject *parent, UtilsNetworking *network, const QString &baseUrl = "https://agoradesk.com/api/v1");

    void countryCodes();
    void currencies();
    void paymentMethods(const QString &countryCode = "");
    void buyMoneroOnline(const QString &currencyCode, const QString &countryCode="", const QString &paymentMethod="", const QString &amount = "", int page = 0);
    void sellMoneroOnline(const QString &currencyCode, const QString &countryCode="", const QString &paymentMethod="", const QString &amount = "", int page = 0);
    void accountInfo(const QString &username);

signals:
    void ApiResponse(LocalMoneroResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);

private:
    QString getBuySellUrl(bool buy, const QString &currencyCode, const QString &countryCode="", const QString &paymentMethod="", const QString &amount = "", int page = 0);

    UtilsNetworking *m_network;
    QString m_baseUrl;
};


#endif //FEATHER_LOCALMONEROAPI_H
