// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "CrowdfundingPlugin.h"

#include "plugins/PluginRegistry.h"
#include "CCSWidget.h"

CrowdfundingPlugin::CrowdfundingPlugin()
{
}

void CrowdfundingPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new CCSWidget(nullptr);
    connect(m_tab, &CCSWidget::fillSendTab, this, &Plugin::fillSendTab);
}

QString CrowdfundingPlugin::id() {
    return "crowdfunding";
}

int CrowdfundingPlugin::idx() const {
    return 10;
}

QString CrowdfundingPlugin::parent() {
    return "home";
}

QString CrowdfundingPlugin::displayName() {
    return "Crowdfunding";
}

QString CrowdfundingPlugin::description() {
    return {};
}

QString CrowdfundingPlugin::icon() {
    return {};
}

QStringList CrowdfundingPlugin::socketData() {
    return {"ccs"};
}

Plugin::PluginType CrowdfundingPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* CrowdfundingPlugin::tab() {
    return m_tab;
}

const bool CrowdfundingPlugin::registered = [] {
    PluginRegistry::registerPlugin(CrowdfundingPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&CrowdfundingPlugin::create);
    return true;
}();
