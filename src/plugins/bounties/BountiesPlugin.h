// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef BOUNTIESPLUGIN_H
#define BOUNTIESPLUGIN_H

#include "plugins/Plugin.h"
#include "BountiesWidget.h"

class BountiesPlugin : public Plugin {
    Q_OBJECT

public:
    explicit BountiesPlugin();

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

    static BountiesPlugin* create() { return new BountiesPlugin(); }

private:
    BountiesWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //BOUNTIESPLUGIN_H
