// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TROCADORAPPAPI_H
#define FEATHER_TROCADORAPPAPI_H

#include <QObject>
#include "utils/Networking.h"

class TrocadorAppApi : public QObject {
    Q_OBJECT

public:
    enum Endpoint {
        CURRENCIES,
        SPREAD,
        BUY_MONERO_ONLINE,
        SELL_MONERO_ONLINE,
        ACCOUNT_INFO
    };

    struct TrocadorAppResponse {
        bool ok;
        Endpoint endpoint;
        QString message;
        QJsonObject obj;
    };

    explicit TrocadorAppApi(QObject *parent, Networking *network);

    void currencies();
    void paymentMethods();
    void buyMoneroOnline(const QString &currencyCode, const QString &paymentMethod="", const QString &amount = "", int page = 0);
    void sellMoneroOnline(const QString &currencyCode, const QString &paymentMethod="", const QString &amount = "", int page = 0);
    void accountInfo(const QString &username);

signals:
    void ApiResponse(TrocadorAppResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);

private:
    QString getBuySellUrl(bool buy, const QString &currencyCode, const QString &paymentMethod="", const QString &amount = "", int page = 0);
    QString getBaseUrl();

    Networking *m_network;
};


#endif //FEATHER_TROCADORAPPAPI_H
