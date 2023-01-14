// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WebsocketClient.h"

#include <QCoreApplication>
#include "utils/Utils.h"

WebsocketClient::WebsocketClient(QObject *parent)
    : QObject(parent)
{
    connect(&webSocket, &QWebSocket::stateChanged, this, &WebsocketClient::onStateChanged);
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

    connect(&m_connectionTimeout, &QTimer::timeout, this, &WebsocketClient::onConnectionTimeout);

    m_websocketUrlIndex = QRandomGenerator::global()->bounded(constants::websocketUrls.length());
    this->nextWebsocketUrl();
}

void WebsocketClient::sendMsg(const QByteArray &data) {
    if (webSocket.state() == QAbstractSocket::ConnectedState) {
        webSocket.sendBinaryMessage(data);
    }
}

void WebsocketClient::start() {
    if (m_stopped) {
        return;
    }

    // connect & reconnect on errors/close
    qDebug() << "WebSocket connect:" << m_url.url();
    auto state = webSocket.state();
    if (state != QAbstractSocket::ConnectedState && state != QAbstractSocket::ConnectingState) {
        webSocket.open(m_url);
    }
}

void WebsocketClient::restart() {
    m_stopped = false;
    this->start();
}

void WebsocketClient::stop() {
    m_stopped = true;
    webSocket.close();
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
    auto state = webSocket.state();
    if (state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState) {
        webSocket.abort();
    }
}

void WebsocketClient::nextWebsocketUrl() {
    m_url = constants::websocketUrls[m_websocketUrlIndex];
    m_websocketUrlIndex = (m_websocketUrlIndex+1)%constants::websocketUrls.length();
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

WebsocketClient::~WebsocketClient() {
    // webSocket may fire QWebSocket::disconnected after WebsocketClient is destroyed
    // explicitly disconnect to prevent crash
    webSocket.disconnect();
}