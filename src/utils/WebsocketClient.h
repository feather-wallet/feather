// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

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
    ~WebsocketClient() override;
    void start();
    void restart();
    void stop();
    void sendMsg(const QByteArray &data);

    QWebSocket webSocket;

signals:
    void connectionEstablished();
    void WSMessage(QJsonObject message);

private slots:
    void onConnected();
    void onDisconnected();
    void onStateChanged(QAbstractSocket::SocketState state);
    void onbinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);
    void nextWebsocketUrl();
    void onConnectionTimeout();

private:
    QUrl m_url;
    QTimer m_pingTimer;
    QTimer m_connectionTimeout;
    int m_timeout = 10;
    int m_websocketUrlIndex = 0;
    bool m_stopped = false;
};

#endif //FEATHER_WEBSOCKETCLIENT_H
