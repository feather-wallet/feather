// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "daemonrpc.h"

#include <utility>

DaemonRpc::DaemonRpc(QObject *parent, UtilsNetworking *network, QString daemonAddress)
        : QObject(parent)
        , m_network(network)
        , m_daemonAddress(std::move(daemonAddress))
{
}

void DaemonRpc::sendRawTransaction(const QString &tx_as_hex, bool do_not_relay, bool do_sanity_checks) {
    QJsonObject req;
    req["tx_as_hex"] = tx_as_hex;
    req["do_not_relay"] = do_not_relay;
    req["do_sanity_checks"] = do_sanity_checks;

    QString url = QString("%1/send_raw_transaction").arg(m_daemonAddress);
    QNetworkReply *reply = m_network->postJson(url, req);
    connect(reply, &QNetworkReply::finished, std::bind(&DaemonRpc::onResponse, this, reply, Endpoint::SEND_RAW_TRANSACTION));
}

void DaemonRpc::onResponse(QNetworkReply *reply, Endpoint endpoint) {
    const auto ok = reply->error() == QNetworkReply::NoError;
    const auto err = reply->errorString();

    QByteArray data = reply->readAll();
    QJsonObject obj;
    if (!data.isEmpty() && Utils::validateJSON(data)) {
        auto doc = QJsonDocument::fromJson(data);
        obj = doc.object();
    }
    else if (!ok) {
        emit ApiResponse(DaemonResponse(false, endpoint, err));
        return;
    }
    else {
        emit ApiResponse(DaemonResponse(false, endpoint, "Invalid response from daemon"));
        return;
    }

    if (obj.value("status").toString() != "OK") {
        QString failedMsg;
        switch (endpoint) {
            case SEND_RAW_TRANSACTION:
                failedMsg = this->onSendRawTransactionFailed(obj);
                break;
            default:
                failedMsg = obj.value("status").toString();
        }

        emit ApiResponse(DaemonResponse(false, endpoint, failedMsg, obj));
        return;
    }

    reply->deleteLater();
    emit ApiResponse(DaemonResponse(true, endpoint, "", obj));
}

QString DaemonRpc::onSendRawTransactionFailed(const QJsonObject &obj) {
    QString message = [&obj]{
        if (obj.value("double_spend").toBool())
            return "Transaction is a double spend";
        if (obj.value("fee_too_low").toBool())
            return "Fee is too low";
        if (obj.value("invalid_input").toBool())
            return "Output is invalid";
        if (obj.value("low_mixin").toBool())
            return "Mixin count is too low";
        if (obj.value("overspend").toBool())
            return "Transaction uses more money than available";
        if (obj.value("too_big").toBool())
            return "Transaction size is too big";
        return "Daemon returned an unknown error";
    }();

    return QString("Transaction failed: %1").arg(message);
}

void DaemonRpc::setDaemonAddress(const QString &daemonAddress) {
    m_daemonAddress = daemonAddress;
}

void DaemonRpc::setNetwork(UtilsNetworking *network) {
    m_network = network;
}