// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WebsocketClient.h"

#include <QCoreApplication>
#include "utils/Utils.h"

WebsocketClient::WebsocketClient(QObject *parent)
    : QObject(parent)
{
    connect(&webSocket, &QWebSocket::connected, this, &WebsocketClient::onConnected);
    connect(&webSocket, &QWebSocket::disconnected, this, &WebsocketClient::onDisconnected);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WebsocketClient::onError);

    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &WebsocketClient::onbinaryMessageReceived);

    // Keep websocket connection alive
    connect(&m_pingTimer, &QTimer::timeout, [this]{
        if (webSocket.state() == QAbstractSocket::ConnectedState) {
            webSocket.ping();
        }
    });
    m_pingTimer.setInterval(30 * 1000);
    m_pingTimer.start();
}

void WebsocketClient::sendMsg(const QByteArray &data) {
    if (webSocket.state() == QAbstractSocket::ConnectedState) {
        webSocket.sendBinaryMessage(data);
    }
}

void WebsocketClient::start() {
    // connect & reconnect on errors/close
    qDebug() << "WebSocket connect:" << m_url.url();

    auto state = webSocket.state();
    if (state != QAbstractSocket::ConnectedState && state != QAbstractSocket::ConnectingState) {
        webSocket.open(m_url);
    }
}

void WebsocketClient::onConnected() {
    qDebug() << "WebSocket connected";
    emit connectionEstablished();
}

void WebsocketClient::onDisconnected() {
    qDebug() << "WebSocket disconnected";
    QTimer::singleShot(1000, [this]{this->start();});
}

void WebsocketClient::onError(QAbstractSocket::SocketError error) {
    qCritical() << "WebSocket error: " << error;
    auto state = webSocket.state();
    if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState) {
        webSocket.abort();
    }
}

void WebsocketClient::onbinaryMessageReceived(const QByteArray &message) {
    qDebug() << "WebSocket received:" << message;

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
