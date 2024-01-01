// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "ExchangePlugin.h"

#include "plugins/PluginRegistry.h"

ExchangePlugin::ExchangePlugin()
{
}

void ExchangePlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new ExchangeWidget(nullptr);
}

QString ExchangePlugin::id() {
    return "exchange";
}

int ExchangePlugin::idx() const {
    return 50;
}

QString ExchangePlugin::parent() {
    return "";
}

QString ExchangePlugin::displayName() {
    return "Exchange";
}

QString ExchangePlugin::description() {
    return {};
}
QString ExchangePlugin::icon() {
    return "update.png";
}

QStringList ExchangePlugin::socketData() {
    return {};
}

Plugin::PluginType ExchangePlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* ExchangePlugin::tab() {
    return m_tab;
}

void ExchangePlugin::addSubPlugin(Plugin* plugin) {
    m_tab->addTab(plugin);
}

bool ExchangePlugin::implicitEnable() {
    return true;
}

const bool ExchangePlugin::registered = [] {
    PluginRegistry::registerPlugin(ExchangePlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&ExchangePlugin::create);
    return true;
}();
