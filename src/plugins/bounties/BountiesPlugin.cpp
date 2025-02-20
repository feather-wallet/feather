// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "BountiesPlugin.h"

#include "plugins/PluginRegistry.h"
#include "BountiesWidget.h"

BountiesPlugin::BountiesPlugin()
{
}

void BountiesPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new BountiesWidget(nullptr);
    connect(m_tab, &BountiesWidget::donate, this, &Plugin::fillSendTab);
}

QString BountiesPlugin::id() {
    return "bounties";
}

int BountiesPlugin::idx() const {
    return 20;
}

QString BountiesPlugin::parent() {
    return "home";
}

QString BountiesPlugin::displayName() {
    return "Bounties";
}

QString BountiesPlugin::description() {
    return "";
}

QString BountiesPlugin::icon() {
    return {};
}

QStringList BountiesPlugin::socketData() {
    return {"bounties"};
}

Plugin::PluginType BountiesPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* BountiesPlugin::tab() {
    return m_tab;
}

const bool BountiesPlugin::registered = [] {
    PluginRegistry::registerPlugin(BountiesPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&BountiesPlugin::create);
    return true;
}();
