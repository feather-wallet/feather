// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef HOMEPLUGIN_H
#define HOMEPLUGIN_H

#include "plugins/Plugin.h"
#include "HomeWidget.h"

class HomePlugin : public Plugin {
    Q_OBJECT

public:
    explicit HomePlugin();

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
    bool insertFirst() override;
    void addSubPlugin(Plugin* plugin) override;

    void initialize(Wallet *wallet, QObject *parent) override;

    static HomePlugin* create() { return new HomePlugin(); }

public slots:
    void aboutToQuit() override;
    void uiSetup() override;

private:
    HomeWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //HOMEPLUGIN_H
