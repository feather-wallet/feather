// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef CALCPLUGIN_H
#define CALCPLUGIN_H

#include "plugins/Plugin.h"
#include "CalcWidget.h"

class CalcPlugin : public Plugin {
    Q_OBJECT

public:
    explicit CalcPlugin();

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

    static CalcPlugin* create() { return new CalcPlugin(); }

public slots:
    void skinChanged() override;

private:
    CalcWidget* m_tab = nullptr;
    static const bool registered;
};


#endif //CALCPLUGIN_H
