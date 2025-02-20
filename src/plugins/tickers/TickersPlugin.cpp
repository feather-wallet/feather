// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TickersPlugin.h"

#include "plugins/PluginRegistry.h"

#include "TickersConfigDialog.h"

TickersPlugin::TickersPlugin()
{
}

void TickersPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new TickersWidget(nullptr, wallet);
}

QString TickersPlugin::id() {
    return "tickers";
}

int TickersPlugin::idx() const {
    return 0;
}

QString TickersPlugin::parent() {
    return "home";
}

QString TickersPlugin::displayName() {
    return "Tickers";
}

QString TickersPlugin::description() {
    return {};
}

QString TickersPlugin::icon() {
    return {};
}

QStringList TickersPlugin::socketData() {
    return {};
}

Plugin::PluginType TickersPlugin::type() {
    return Plugin::PluginType::WIDGET;
}

QWidget* TickersPlugin::tab() {
    return m_tab;
}

bool TickersPlugin::configurable() {
    return true;
}

QDialog* TickersPlugin::configDialog(QWidget *parent) {
    return new TickersConfigDialog{parent};
}

const bool TickersPlugin::registered = [] {
    PluginRegistry::registerPlugin(TickersPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&TickersPlugin::create);
    return true;
}();
