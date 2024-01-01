// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef REVUOPLUGIN_H
#define REVUOPLUGIN_H

#include "plugins/Plugin.h"
#include "RevuoWidget.h"

class RevuoPlugin : public Plugin {
    Q_OBJECT

public:
    explicit RevuoPlugin();

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

    static RevuoPlugin* create() { return new RevuoPlugin(); }

public slots:
    void skinChanged() override;

private:
    RevuoWidget* m_tab = nullptr;
    static const bool registered;
};

#endif //REVUOPLUGIN_H
