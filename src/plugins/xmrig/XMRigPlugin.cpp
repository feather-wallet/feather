// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "XMRigPlugin.h"

#include "plugins/PluginRegistry.h"

XMRigPlugin::XMRigPlugin()
{
}


void XMRigPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new XMRigWidget(wallet, nullptr);
}

QString XMRigPlugin::id() {
    return "xmrig";
}

int XMRigPlugin::idx() const {
    return 70;
}

QString XMRigPlugin::parent() {
    return {};
}

QString XMRigPlugin::displayName() {
    return "Mining";
}

QString XMRigPlugin::description() {
    return {};
}

QString XMRigPlugin::icon() {
    return "mining.png";
}

QStringList XMRigPlugin::socketData() {
    return {"xmrig"};
}

Plugin::PluginType XMRigPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* XMRigPlugin::tab() {
    return m_tab;
}

bool XMRigPlugin::requiresWebsocket() {
    return false;
}

const bool XMRigPlugin::registered = [] {
    PluginRegistry::registerPlugin(XMRigPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&XMRigPlugin::create);
    return true;
}();
