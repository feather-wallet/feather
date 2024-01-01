// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "RevuoPlugin.h"

#include "plugins/PluginRegistry.h"
#include "RevuoWidget.h"

RevuoPlugin::RevuoPlugin()
{
}

void RevuoPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new RevuoWidget(nullptr);
    connect(m_tab, &RevuoWidget::donate, this, &Plugin::fillSendTab);
}

QString RevuoPlugin::id() {
    return "revuo";
}

int RevuoPlugin::idx() const {
    return 40;
}

QString RevuoPlugin::parent() {
    return "home";
}

QString RevuoPlugin::displayName() {
    return "Revuo";
}

QString RevuoPlugin::description() {
    return {};
}

QString RevuoPlugin::icon() {
    return {};
}

QStringList RevuoPlugin::socketData() {
    return {"revuo"};
}

Plugin::PluginType RevuoPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* RevuoPlugin::tab() {
    return m_tab;
}

void RevuoPlugin::skinChanged() {
    m_tab->skinChanged();
}

const bool RevuoPlugin::registered = [] {
    PluginRegistry::registerPlugin(RevuoPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&RevuoPlugin::create);
    return true;
}();
