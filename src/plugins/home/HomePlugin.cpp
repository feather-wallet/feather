// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "HomePlugin.h"

#include "plugins/PluginRegistry.h"

HomePlugin::HomePlugin()
{
}

void HomePlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new HomeWidget(nullptr);
}

QString HomePlugin::id() {
    return "home";
}

int HomePlugin::idx() const {
    return 0;
}

QString HomePlugin::parent() {
    return "";
}

QString HomePlugin::displayName() {
    return "Home";
}

QString HomePlugin::description() {
    return {};
}
QString HomePlugin::icon() {
    return "tab_home.png";
}

QStringList HomePlugin::socketData() {
    return {};
}

Plugin::PluginType HomePlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* HomePlugin::tab() {
    return m_tab;
}

void HomePlugin::addSubPlugin(Plugin* plugin) {
    m_tab->addPlugin(plugin);
}

bool HomePlugin::implicitEnable() {
    return true;
}

bool HomePlugin::insertFirst() {
    return true;
}

void HomePlugin::aboutToQuit() {
    m_tab->aboutToQuit();
}

void HomePlugin::uiSetup() {
    m_tab->uiSetup();
}

const bool HomePlugin::registered = [] {
    PluginRegistry::registerPlugin(HomePlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&HomePlugin::create);
    return true;
}();
