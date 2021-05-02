// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "NetworkManager.h"

#include <QCoreApplication>
#include <QNetworkProxy>

QNetworkAccessManager *g_networkManagerTor = nullptr;
QNetworkAccessManager *g_networkManagerClearnet = nullptr;

QNetworkAccessManager* getNetworkTor()
{
    if (!g_networkManagerTor) {
        g_networkManagerTor = new QNetworkAccessManager(QCoreApplication::instance());
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setHostName("127.0.0.1");
        proxy.setPort(9050);
        g_networkManagerTor->setProxy(proxy);
    }
    return g_networkManagerTor;
}

QNetworkAccessManager* getNetworkClearnet()
{
    if (!g_networkManagerClearnet) {
        g_networkManagerClearnet = new QNetworkAccessManager(QCoreApplication::instance());
    }
    return g_networkManagerClearnet;
}

//void setTorProxy(const QNetworkProxy &proxy)
//{
//    QNetworkAccessManager *network = getNetworkTor();
//    network->setProxy(proxy);
//}