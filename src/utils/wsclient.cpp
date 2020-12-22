// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QObject>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QScreen>
#include <QDesktopWidget>
#include <QtCore>
#include "wsclient.h"
#include "appcontext.h"


WSClient::WSClient(AppContext *ctx, const QUrl &url, QObject *parent) :
        QObject(parent),
        url(url),
        m_ctx(ctx) {
    connect(&this->webSocket, &QWebSocket::binaryMessageReceived, this, &WSClient::onbinaryMessageReceived);
    connect(&this->webSocket, &QWebSocket::connected, this, &WSClient::onConnected);
    connect(&this->webSocket, &QWebSocket::disconnected, this, &WSClient::closed);
    connect(&this->webSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &WSClient::onError);

    m_tor = url.host().endsWith(".onion");

    // Keep websocket connection alive
    connect(&m_pingTimer, &QTimer::timeout, [this]{
        if (this->webSocket.state() == QAbstractSocket::ConnectedState)
            this->webSocket.ping();
    });
    m_pingTimer.setInterval(30 * 1000);
    m_pingTimer.start();
}

void WSClient::sendMsg(const QByteArray &data) {
    auto state = this->webSocket.state();
    if(state == QAbstractSocket::ConnectedState)
        this->webSocket.sendBinaryMessage(data);
}

void WSClient::start() {
    // connect & reconnect on errors/close
#ifdef QT_DEBUG
    qDebug() << "WebSocket connect:" << url.url();
#endif
    if((m_tor && this->m_ctx->tor->torConnected) || !m_tor)
        this->webSocket.open(QUrl(this->url));

    if(!this->m_connectionTimer.isActive()) {
        connect(&this->m_connectionTimer, &QTimer::timeout, this, &WSClient::checkConnection);
        this->m_connectionTimer.start(2000);
    }
}

void WSClient::checkConnection() {
    if(m_tor && !this->m_ctx->tor->torConnected)
        return;

    auto state = this->webSocket.state();
    if(state == QAbstractSocket::UnconnectedState) {
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
    auto state = this->webSocket.state();
    if(state == QAbstractSocket::ConnectedState || state == QAbstractSocket::ConnectingState)
        this->webSocket.abort();
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
