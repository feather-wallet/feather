// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "LocalMoneroPlugin.h"

LocalMoneroPlugin::LocalMoneroPlugin()
{
}

void LocalMoneroPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new LocalMoneroWidget(nullptr, wallet);
}

QString LocalMoneroPlugin::id() {
    return "localmonero";
}

int LocalMoneroPlugin::idx() const {
    return 0;
}

QString LocalMoneroPlugin::parent() {
    return "exchange";
}

QString LocalMoneroPlugin::displayName() {
    return "LocalMonero";
}

QString LocalMoneroPlugin::description() {
    return {};
}
QString LocalMoneroPlugin::icon() {
    return "localMonero_logo.png";
}

QStringList LocalMoneroPlugin::socketData() {
    return {"localmonero_countries", "localmonero_currencies", "localmonero_payment_methods"};
}

Plugin::PluginType LocalMoneroPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* LocalMoneroPlugin::tab() {
    return m_tab;
}

void LocalMoneroPlugin::skinChanged() {
    m_tab->skinChanged();
}

const bool LocalMoneroPlugin::registered = [] {
    PluginRegistry::registerPlugin(LocalMoneroPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&LocalMoneroPlugin::create);
    return true;
}();
