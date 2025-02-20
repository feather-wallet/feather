// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "NetworkManager.h"

#include <QCoreApplication>
#include <QNetworkProxy>
#include <QUrl>

#include "utils/config.h"
#include "utils/Utils.h"

QNetworkAccessManager *g_networkManagerSocks5 = nullptr;
QNetworkAccessManager *g_networkManagerClearnet = nullptr;

QNetworkAccessManager* getNetworkSocks5()
{
    if (!g_networkManagerSocks5) {
        g_networkManagerSocks5 = new QNetworkAccessManager(QCoreApplication::instance());
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::Socks5Proxy);
        proxy.setHostName("127.0.0.1");
        proxy.setPort(9050);
        g_networkManagerSocks5->setProxy(proxy);
    }
    return g_networkManagerSocks5;
}

QNetworkAccessManager* getNetworkClearnet()
{
    if (!g_networkManagerClearnet) {
        g_networkManagerClearnet = new QNetworkAccessManager(QCoreApplication::instance());
    }
    return g_networkManagerClearnet;
}


QNetworkAccessManager* getNetwork(const QString &address)
{
    if (conf()->get(Config::proxy).toInt() == Config::Proxy::None) {
        return getNetworkClearnet();
    }

    // Ignore proxy rules for local addresses
    if (!address.isEmpty() && Utils::isLocalUrl(QUrl(address))) {
        return getNetworkClearnet();
    }

    return getNetworkSocks5();
}