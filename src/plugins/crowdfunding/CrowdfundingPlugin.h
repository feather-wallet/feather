// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef CROWDFUNDINGPLUGIN_H
#define CROWDFUNDINGPLUGIN_H

#include "plugins/Plugin.h"
#include "CCSWidget.h"

class CrowdfundingPlugin : public Plugin {
    Q_OBJECT

public:
    explicit CrowdfundingPlugin();

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

    static CrowdfundingPlugin* create() { return new CrowdfundingPlugin(); }

private:
    CCSWidget* m_tab = nullptr;
    static const bool registered;
};

#endif //CROWDFUNDINGPLUGIN_H
