// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_WEBSOCKETCLIENT_H
#define FEATHER_WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QPointer>
#include "constants.h"
#include "utils/config.h"

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
    void onConnectionTimeout();

private:
    Config::Proxy networkType();
    const QHash<Config::Proxy, QVector<QUrl>> m_websocketUrls = {
            {Config::Proxy::None, {
                        QUrl(QStringLiteral("wss://ws.featherwallet.org/ws")),
                        QUrl(QStringLiteral("wss://ws.featherwallet.net/ws"))
                }},
            {Config::Proxy::Tor, {
                        QUrl(QStringLiteral("ws://7e6egbawekbkxzkv4244pqeqgoo4axko2imgjbedwnn6s5yb6b7oliqd.onion/ws")),
                        QUrl(QStringLiteral("ws://an5ecwgzyujqe7jverkp42d22zhvjes2mrhvol6tpqcgfkzwseqrafqd.onion/ws"))
                }},
            {Config::Proxy::i2p, {
                        QUrl(QStringLiteral("ws://hk5smvnkifjcm5346bs6cmnczwbiupr4jyiw3gz5z7ybaigp72fa.b32.i2p/ws")),
                        QUrl(QStringLiteral("ws://tr7iahturgfii64txw7cjhrfunmpg35w2lmmqmsa6i4jxwi7vplq.b32.i2p/ws"))
                }}
    };

    QUrl m_url;
    QTimer m_pingTimer;
    QTimer m_connectionTimeout;
    int m_timeout = 20;
    int m_websocketUrlIndex = 0;
    bool m_stopped = false;
};

#endif //FEATHER_WEBSOCKETCLIENT_H
