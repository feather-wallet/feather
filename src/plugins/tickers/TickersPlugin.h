// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TICKERPLUGIN_H
#define TICKERPLUGIN_H

#include "plugins/Plugin.h"
#include "TickersWidget.h"

class TickersPlugin : public Plugin {
    Q_OBJECT

public:
    explicit TickersPlugin();

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

    static TickersPlugin* create() { return new TickersPlugin(); }

private:
    TickersWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //TICKERPLUGIN_H
