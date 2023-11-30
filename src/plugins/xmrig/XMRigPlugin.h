// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef XMRIGPLUGIN_H
#define XMRIGPLUGIN_H

#include "plugins/Plugin.h"
#include "XMRigWidget.h"

class XMRigPlugin : public Plugin {
    Q_OBJECT

public:
    explicit XMRigPlugin();

    QString id() override;
    int idx() const override;
    QString parent() override;
    QString displayName() override;
    QString description() override;
    QString icon() override;
    QStringList socketData() override;
    PluginType type() override;
    QWidget* tab() override;
    bool requiresWebsocket() override;

    void initialize(Wallet *wallet, QObject *parent) override;

    static XMRigPlugin* create() { return new XMRigPlugin(); }

private:
    XMRigWidget* m_tab = nullptr;
    static const bool registered;
};

#endif //XMRIGPLUGIN_H
