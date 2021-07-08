// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QNetworkAccessManager>
#include <utility>
#include "wsclient.h"
#include "utils/Utils.h"

WSClient::WSClient(QUrl url, QObject *parent)
    : QObject(parent)
    , m_url(std::move(url))
{
    connect(&webSocket, &QWebSocket::binaryMessageReceived, this, &WSClient::onbinaryMessageReceived);
    connect(&webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(&webSocket, &QWebSocket::disconnected, this, &WSClient::closed);
    connect(&webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WSClient::onError);
    connect(&m_connectionTimer, &QTimer::timeout, this, &WSClient::checkConnection);

    // Keep websocket connection alive
    connect(&m_pingTimer, &QTimer::timeout, [this]{
        if (webSocket.state() == QAbstractSocket::ConnectedState)
            webSocket.ping();
    });
    m_pingTimer.setInterval(30 * 1000);
    m_pingTimer.start();
}

void WSClient::sendMsg(const QByteArray &data) {
    if (webSocket.state() == QAbstractSocket::ConnectedState)
        webSocket.sendBinaryMessage(data);
}

void WSClient::onToggleConnect(bool connect) {
    m_connect = connect;
    if (m_connect)
        checkConnection();
}

void WSClient::start() {
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

void WSClient::checkConnection() {
    if (!m_connect)
        return;

    if (webSocket.state() == QAbstractSocket::UnconnectedState) {
#ifdef QT_DEBUG
        qDebug() << "WebSocket reconnect";
#endif
        this->start();
    }
}

void WSClient::onConnected() {
#ifdef QT_DEBUG
    qDebug() << "WebSocket connected";
#endif
    emit connectionEstablished();
}

void WSClient::onError(QAbstractSocket::SocketError error) {
    qCritical() << "WebSocket error: " << error;
    auto state = webSocket.state();
    if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState)
        webSocket.abort();
}

void WSClient::onbinaryMessageReceived(const QByteArray &message) {
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
