// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef LOCALMONEROPLUGIN_H
#define LOCALMONEROPLUGIN_H

#include "plugins/Plugin.h"
#include "LocalMoneroWidget.h"
#include "plugins/PluginRegistry.h"

class LocalMoneroPlugin : public Plugin {
    Q_OBJECT

public:
    explicit LocalMoneroPlugin();

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

    static LocalMoneroPlugin* create() { return new LocalMoneroPlugin(); }

public slots:
    void skinChanged() override;

private:
    LocalMoneroWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //LOCALMONEROPLUGIN_H
