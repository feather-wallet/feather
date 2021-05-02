// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WebsocketClient.h"

#include <QCoreApplication>
#include "utils/utils.h"

WebsocketClient::WebsocketClient(QObject *parent)
    : QObject(parent)
{
    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &WebsocketClient::onbinaryMessageReceived);
    connect(&webSocket, &QWebSocket::connected, this, &WebsocketClient::onConnected);
    connect(&webSocket, &QWebSocket::disconnected, this, &WebsocketClient::closed);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebsocketClient::onError);
    connect(&m_connectionTimer, &QTimer::timeout, this, &WebsocketClient::checkConnection);

    // Keep websocket connection alive
    connect(&m_pingTimer, &QTimer::timeout, [this]{
        if (webSocket.state() == QAbstractSocket::ConnectedState)
            webSocket.ping();
    });
    m_pingTimer.setInterval(30 * 1000);
    m_pingTimer.start();
}

void WebsocketClient::sendMsg(const QByteArray &data) {
    if (webSocket.state() == QAbstractSocket::ConnectedState)
        webSocket.sendBinaryMessage(data);
}

void WebsocketClient::onToggleConnect(bool connect) {
    m_connect = connect;
    if (m_connect)
        checkConnection();
}

void WebsocketClient::start() {
    // connect & reconnect on errors/close
#ifdef QT_DEBUG
    qDebug() << "WebSocket connect:" << m_url.url();
#endif

    if (m_connect)
        webSocket.open(m_url);

    if (!m_connectionTimer.isActive()) {
        m_connectionTimer.start(2000);
    }
}

void WebsocketClient::checkConnection() {
    if (!m_connect)
        return;

    if (webSocket.state() == QAbstractSocket::UnconnectedState) {
#ifdef QT_DEBUG
        qDebug() << "WebSocket reconnect";
#endif
        this->start();
    }
}

void WebsocketClient::onConnected() {
#ifdef QT_DEBUG
    qDebug() << "WebSocket connected";
#endif
    emit connectionEstablished();
}

void WebsocketClient::onError(QAbstractSocket::SocketError error) {
    qCritical() << "WebSocket error: " << error;
    auto state = webSocket.state();
    if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState)
        webSocket.abort();
}

void WebsocketClient::onbinaryMessageReceived(const QByteArray &message) {
#ifdef QT_DEBUG
    qDebug() << "WebSocket received:" << message;
#endif
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
