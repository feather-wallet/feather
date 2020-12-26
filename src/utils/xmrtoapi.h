// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_XMRTOAPI_H
#define FEATHER_XMRTOAPI_H

#include <QObject>
#include <utility>

#include "utils/networking.h"

enum Endpoint {
    RATES = 0,
    ORDER_CREATE,
    ORDER_STATUS
};

struct XmrToError {
    explicit XmrToError(QString code = "", QString msg = "")
        : code(std::move(code)), msg(std::move(msg)) {};

    QString code;
    QString msg;
};

struct XmrToResponse {
    explicit XmrToResponse(bool ok, Endpoint endpoint, QString message, XmrToError error = XmrToError(), QJsonObject obj = {})
        : ok(ok), endpoint(endpoint), message(std::move(message)), error(std::move(error)), obj(std::move(obj)) {};

    explicit XmrToResponse(bool ok, Endpoint endpoint, QString message, QJsonObject obj)
        : ok(ok), endpoint(endpoint), message(std::move(message)), obj(std::move(obj)) {};

    bool ok;
    Endpoint endpoint;
    QString message;
    XmrToError error = XmrToError();
    QJsonObject obj;
};

class XmrToApi : public QObject {
    Q_OBJECT

public:
    explicit XmrToApi(QObject *parent, UtilsNetworking *network, QString baseUrl = "https://xmr.to");

    void getRates();
    void createOrder(double amount, const QString &amount_currency, const QString &dest_address);
    void getOrderStatus(const QString &uuid);

signals:
    void ApiResponse(XmrToResponse resp);

private slots:
    void onResponse(QNetworkReply *reply, Endpoint endpoint);

private:
    static XmrToError getApiError(const QJsonObject &obj);

    QString m_baseUrl;
    UtilsNetworking *m_network;

    // https://xmrto-api.readthedocs.io/en/latest/introduction.html#list-of-all-error-codes
    const QMap<QString, QString> m_errorMap = {
            {"XMRTO-ERROR-001", "internal services not available, try again later."},
            {"XMRTO-ERROR-002", "malformed bitcoin address, check address validity."},
            {"XMRTO-ERROR-003", "invalid bitcoin amount, check amount data type."},
            {"XMRTO-ERROR-004", "bitcoin amount out of bounds, check min and max amount."},
            {"XMRTO-ERROR-005", "unexpected validation error, contact support."},
            {"XMRTO-ERROR-006", "requested order not found, check order UUID."},
            {"XMRTO-ERROR-007", "third party service not available, try again later."},
            {"XMRTO-ERROR-008", "insufficient funds available, try again later."},
            {"XMRTO-ERROR-009", "invalid request, check request parameters."},
            {"XMRTO-ERROR-010", "payment protocol failed, invalid or outdated data served by URL."},
            {"XMRTO-ERROR-011", "malformed payment protocol url, URL is malformed or cannot be contacted."},
            {"XMRTO-ERROR-012", "too many requests, try less often."},
            {"XMRTO-ERROR-013", "access forbidden."},
            {"XMRTO-ERROR-014", "service is not available in your region."},
            {"XMRTO-ERROR-015", "invalid monero amount, check amount data type."},
            {"XMRTO-ERROR-016", "invalid currency, check available currency options."},
            {"XMRTO-ERROR-017", "malformed lightning network invoice, provide a correct invoice for the main network."},
            {"XMRTO-ERROR-018", "lightning payment unlikely to succeed, check first if xmr.to has routes available."},
            {"XMRTO-ERROR-019", "lightning invoice preimage already known, don’t use the same invoice more than once."}
    };
};


#endif //FEATHER_XMRTOAPI_H
