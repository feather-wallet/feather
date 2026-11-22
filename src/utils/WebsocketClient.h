// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WEBSOCKETCLIENT_H
#define FEATHER_WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QVector>

class WebsocketClient : public QObject {
    Q_OBJECT

public:
    explicit WebsocketClient(QObject *parent = nullptr);
    ~WebsocketClient() override;
    void start();
    void restart();
    void stop();
    void sendMsg(const QByteArray &data);
    void nextWebsocketUrl();

    QWebSocket *webSocket;

signals:
    void connectionEstablished();
    void WSMessage(QJsonObject message);

private slots:
    void onConnected();
    void onDisconnected();
    void onStateChanged(QAbstractSocket::SocketState state);
    void onbinaryMessageReceived(const QByteArray &message);
    void onError(QAbstractSocket::SocketError error);
    void onConnectionTimeout();

private:
    const QVector<QUrl> m_websocketUrls = {
        QUrl(QStringLiteral("wss://ws.featherwallet.org/ws")),
        QUrl(QStringLiteral("wss://ws.featherwallet.net/ws"))
    };

    QUrl m_url;
    QTimer m_pingTimer;
    QTimer m_connectionTimeout;
    int m_timeout = 20;
    int m_websocketUrlIndex = 0;
    bool m_stopped = false;
};

#endif //FEATHER_WEBSOCKETCLIENT_H
