// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_NETWORKPROXYWIDGET_H
#define FEATHER_NETWORKPROXYWIDGET_H

#include <QWidget>
#include <QTextEdit>

#include "dialog/TorInfoDialog.h"

namespace Ui {
    class NetworkProxyWidget;
}

class NetworkProxyWidget : public QWidget
{
Q_OBJECT

public:
    explicit NetworkProxyWidget(QWidget *parent = nullptr);
    ~NetworkProxyWidget() override;

    void setProxySettings();
    bool isProxySettingsChanged() {
        return m_proxySettingsChanged;
    };
    void setDisableTorLogs();

signals:
    void proxySettingsChanged();

private:
    void onProxySettingsChanged();
    void updatePort();

    QScopedPointer<Ui::NetworkProxyWidget> ui;
    TorInfoDialog *m_torInfoDialog;

    bool m_disableTorLogs = false;
    bool m_proxySettingsChanged = false;
};


#endif //FEATHER_NETWORKPROXYWIDGET_H
