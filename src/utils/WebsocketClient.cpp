// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "WebsocketClient.h"

#include <QCoreApplication>
#include "utils/Utils.h"

#include "utils/config.h"

WebsocketClient::WebsocketClient(QObject *parent)
    : QObject(parent)
    , webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
{
    connect(webSocket, &QWebSocket::stateChanged, this, &WebsocketClient::onStateChanged);
    connect(webSocket, &QWebSocket::connected, this, &WebsocketClient::onConnected);
    connect(webSocket, &QWebSocket::disconnected, this, &WebsocketClient::onDisconnected);
    connect(webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebsocketClient::onError);

    connect(webSocket, &QWebSocket::binaryMessageReceived, this, &WebsocketClient::onbinaryMessageReceived);

    // Keep websocket connection alive
    connect(&m_pingTimer, &QTimer::timeout, [this]{
        if (webSocket->state() == QAbstractSocket::ConnectedState) {
            webSocket->ping();
        }
    });
    m_pingTimer.setInterval(30 * 1000);
    m_pingTimer.start();

    connect(&m_connectionTimeout, &QTimer::timeout, this, &WebsocketClient::onConnectionTimeout);

    m_websocketUrlIndex = QRandomGenerator::global()->bounded(m_websocketUrls[this->networkType()].length());
    this->nextWebsocketUrl();
}

void WebsocketClient::sendMsg(const QByteArray &data) {
    if (webSocket->state() == QAbstractSocket::ConnectedState) {
        webSocket->sendBinaryMessage(data);
    }
}

void WebsocketClient::start() {
    if (m_stopped) {
        return;
    }

    if (conf()->get(Config::offlineMode).toBool()) {
        return;
    }

    if (conf()->get(Config::disableWebsocket).toBool()) {
        return;
    }

    // connect & reconnect on errors/close
    auto state = webSocket->state();
    if (state != QAbstractSocket::ConnectedState && state != QAbstractSocket::ConnectingState) {
        qDebug() << "WebSocket connect:" << m_url.url();
        webSocket->open(m_url);
    }
}

void WebsocketClient::restart() {
    m_stopped = false;
    this->start();
}

void WebsocketClient::stop() {
    m_stopped = true;
    webSocket->close();
    m_connectionTimeout.stop();
}

void WebsocketClient::onConnected() {
    qDebug() << "WebSocket connected";
    emit connectionEstablished();
}

void WebsocketClient::onDisconnected() {
    qDebug() << "WebSocket disconnected";
    this->nextWebsocketUrl();
    QTimer::singleShot(1000, [this]{this->start();});
}

void WebsocketClient::onStateChanged(QAbstractSocket::SocketState state) {
    if (state == QAbstractSocket::ConnectingState) {
        m_connectionTimeout.start(m_timeout*1000);
    }
    else if (state == QAbstractSocket::ConnectedState) {
        m_connectionTimeout.stop();
    }
}

void WebsocketClient::onError(QAbstractSocket::SocketError error) {
    qCritical() << "WebSocket error: " << error;
    auto state = webSocket->state();
    if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState) {
        webSocket->abort();
    }
}

void WebsocketClient::nextWebsocketUrl() {
    Config::Proxy networkType = this->networkType();
    m_websocketUrlIndex = (m_websocketUrlIndex+1)%m_websocketUrls[networkType].length();
    m_url = m_websocketUrls[networkType][m_websocketUrlIndex];
}

Config::Proxy WebsocketClient::networkType() {
    if (conf()->get(Config::proxy).toInt() == Config::Proxy::Tor && conf()->get(Config::torOnlyAllowOnion).toBool()) {
        // Websocket performance with onion services is abysmal, connect to clearnet server unless instructed otherwise
        return Config::Proxy::Tor;
    }
    else if (conf()->get(Config::proxy).toInt() == Config::Proxy::i2p) {
        return Config::Proxy::i2p;
    }
    else {
        return Config::Proxy::None;
    }
}

void WebsocketClient::onConnectionTimeout() {
    qWarning() << "Websocket connection timeout";
    m_timeout = std::min(m_timeout + 5, 60);
    m_connectionTimeout.setInterval(m_timeout*1000);
    this->onDisconnected();
}

void WebsocketClient::onbinaryMessageReceived(const QByteArray &message) {
//    qDebug() << "WebSocket received:" << message;

    if (!Utils::validateJSON(message)) {
        qCritical() << "Could not interpret WebSocket message as JSON";
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject object = doc.object();
    if(!object.contains("cmd") || !object.contains("data")) {
        qCritical() << "Invalid WebSocket message received";
        return;
    }

    emit WSMessage(object);
}

WebsocketClient::~WebsocketClient() = default;