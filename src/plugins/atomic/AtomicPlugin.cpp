// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicPlugin.h"
#include "AtomicConfigDialog.h"

#include "plugins/PluginRegistry.h"

AtomicPlugin::AtomicPlugin()
{
}

void AtomicPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new AtomicWidget(nullptr);
}

QString AtomicPlugin::id() {
    return "atomic";
}

int AtomicPlugin::idx() const {
    return 61;
}

QString AtomicPlugin::parent() {
    return {};
}

QString AtomicPlugin::displayName() {
    return "Atomic";
}

QString AtomicPlugin::description() {
    return {};
}

QString AtomicPlugin::icon() {
    return "atomic-icon.png";
}

QStringList AtomicPlugin::socketData() {
    return {};
}

Plugin::PluginType AtomicPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* AtomicPlugin::tab() {
    return m_tab;
}

bool AtomicPlugin::configurable() {
    return true;
}

QDialog* AtomicPlugin::configDialog(QWidget *parent) {
    return new AtomicConfigDialog{parent};
}

void AtomicPlugin::skinChanged() {
    m_tab->skinChanged();
}

const bool AtomicPlugin::registered = [] {
    PluginRegistry::registerPlugin(AtomicPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&AtomicPlugin::create);
    return true;
}();
