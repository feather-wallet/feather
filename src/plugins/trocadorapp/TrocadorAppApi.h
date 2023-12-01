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
        REQUEST_STANDARD,
        REQUEST_PAYMENT
    };

    struct TrocadorAppResponse {
        bool ok;
        Endpoint endpoint;
        QString message;
        QJsonObject obj;
    };

    explicit TrocadorAppApi(QObject *parent, Networking *network);

    void requestStandard(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                         const QString &networkTo, const QString &amountFrom, const QString &paymentMethod);
    void requestPayment(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                         const QString &networkTo, const QString &amountTo, const QString &paymentMethod);
    QString getBaseUrl();

signals:
    void ApiResponse(TrocadorAppResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);

private:
    QString getStandardPaymentUrl(const QString &currencyCode, const QString &networkFrom, const QString &tradeForCode,
                                  const QString &networkTo, const QString &amount, const QString &paymentMethod);

    Networking *m_network;
};


#endif //FEATHER_TROCADORAPPAPI_H
