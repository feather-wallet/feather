// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "CalcPlugin.h"
#include "CalcConfigDialog.h"

#include "plugins/PluginRegistry.h"

CalcPlugin::CalcPlugin()
{
}

void CalcPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_tab = new CalcWidget(nullptr);
}

QString CalcPlugin::id() {
    return "calc";
}

int CalcPlugin::idx() const {
    return 60;
}

QString CalcPlugin::parent() {
    return {};
}

QString CalcPlugin::displayName() {
    return "Calc";
}

QString CalcPlugin::description() {
    return {};
}

QString CalcPlugin::icon() {
    return "gnome-calc.png";
}

QStringList CalcPlugin::socketData() {
    return {};
}

Plugin::PluginType CalcPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* CalcPlugin::tab() {
    return m_tab;
}

bool CalcPlugin::configurable() {
    return true;
}

QDialog* CalcPlugin::configDialog(QWidget *parent) {
    return new CalcConfigDialog{parent};
}

void CalcPlugin::skinChanged() {
    m_tab->skinChanged();
}

const bool CalcPlugin::registered = [] {
    PluginRegistry::registerPlugin(CalcPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&CalcPlugin::create);
    return true;
}();
