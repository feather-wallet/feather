// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef EXCHANGEPLUGIN_H
#define EXCHANGEPLUGIN_H

#include "plugins/Plugin.h"
#include "ExchangeWidget.h"

class ExchangePlugin : public Plugin {
    Q_OBJECT

public:
    explicit ExchangePlugin();

    QString id() override;
    int idx() const override;
    QString parent() override;
    QString displayName() override;
    QString description() override;
    QString icon() override;
    QStringList socketData() override;
    PluginType type() override;
    QWidget* tab() override;
    bool implicitEnable() override;
    void addSubPlugin(Plugin* plugin) override;

    void initialize(Wallet *wallet, QObject *parent) override;

    static ExchangePlugin* create() { return new ExchangePlugin(); }

private:
    ExchangeWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //EXCHANGEPLUGIN_H
