// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "RedditPlugin.h"

#include "plugins/PluginRegistry.h"
#include "RedditWidget.h"

RedditPlugin::RedditPlugin()
{
}

void RedditPlugin::initialize(Wallet *wallet, QObject *parent) {
    this->setParent(parent);
    m_redditWidget = new RedditWidget(nullptr);
    connect(m_redditWidget, &RedditWidget::setStatusText, this, &Plugin::setStatusText);
}

QString RedditPlugin::id() {
    return "reddit";
}

int RedditPlugin::idx() const {
    return 30;
}

QString RedditPlugin::parent() {
    return "home";
}

QString RedditPlugin::displayName() {
    return "Reddit";
}

QString RedditPlugin::description() {
    return {};
}

QString RedditPlugin::icon() {
    return {};
}

QStringList RedditPlugin::socketData() {
    return {"reddit"};
}

Plugin::PluginType RedditPlugin::type() {
    return Plugin::PluginType::TAB;
}

QWidget* RedditPlugin::tab() {
    return m_redditWidget;
}

const bool RedditPlugin::registered = [] {
    PluginRegistry::registerPlugin(RedditPlugin::create());
    PluginRegistry::getInstance().registerPluginCreator(&RedditPlugin::create);
    return true;
}();
