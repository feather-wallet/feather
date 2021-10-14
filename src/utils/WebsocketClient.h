// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WEBSOCKETCLIENT_H
#define FEATHER_WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QPointer>
#include "constants.h"

class WebsocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebsocketClient(QObject *parent = nullptr);
    void start();
    void sendMsg(const QByteArray &data);

    QWebSocket webSocket;

signals:
    void connectionEstablished();
    void WSMessage(QJsonObject message);

private slots:
    void onConnected();
    void onDisconnected();
    void onbinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);

private:
    QUrl m_url = constants::websocketUrl;
    QTimer m_pingTimer;
};

#endif //FEATHER_WEBSOCKETCLIENT_H
