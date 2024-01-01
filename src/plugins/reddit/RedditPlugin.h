// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef REDDITPLUGIN_H
#define REDDITPLUGIN_H

#include "plugins/Plugin.h"
#include "RedditWidget.h"
#include "plugins/PluginRegistry.h"

class RedditPlugin : public Plugin {
    Q_OBJECT

public:
    explicit RedditPlugin();

    QString id() override;
    int idx() const override;
    QString parent() override;
    QString displayName() override;
    QString description() override;
    QString icon() override;
    QStringList socketData() override;
    PluginType type() override;
    QWidget* tab() override;

    void initialize(Wallet *wallet, QObject *parent) override;

    static RedditPlugin* create() { return new RedditPlugin(); }

private:
    RedditWidget* m_redditWidget = nullptr;
    static const bool registered;
};




#endif //REDDITPLUGIN_H
