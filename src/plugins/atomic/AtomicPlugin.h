// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef ATOMICPLUGIN_H
#define ATOMICPLUGIN_H

#include "plugins/Plugin.h"
#include "AtomicWidget.h"

class AtomicPlugin : public Plugin {
Q_OBJECT

public:
    explicit AtomicPlugin();

    QString id() override;
    int idx() const override;
    QString parent() override;
    QString displayName() override;
    QString description() override;
    QString icon() override;
    QStringList socketData() override;
    PluginType type() override;
    QWidget* tab() override;
    bool configurable() override;
    QDialog* configDialog(QWidget *parent) override;

    void initialize(Wallet *wallet, QObject *parent) override;

    static AtomicPlugin* create() { return new AtomicPlugin(); }

public slots:
    void skinChanged() override;

private:
    AtomicWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //ATOMICPLUGIN_H
