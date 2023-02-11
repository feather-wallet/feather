// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_SETTINGSNEWDIALOG_H
#define FEATHER_SETTINGSNEWDIALOG_H


#include <QAbstractButton>
#include <QDialog>
#include <QSettings>

#include "appcontext.h"
#include "widgets/NodeWidget.h"

namespace Ui {
    class SettingsNew;
}

class SettingsNew : public QDialog
{
Q_OBJECT

public:
    explicit SettingsNew(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~SettingsNew() override;

    void showNetworkProxyTab();

    enum Pages {
        APPEARANCE = 0,
        NETWORK,
        STORAGE,
        DISPLAY,
        MEMORY,
        TRANSACTIONS,
        MISC
    };

signals:
    void preferredFiatCurrencyChanged(QString currency);
    void skinChanged(QString skinName);
    void blockExplorerChanged(QString blockExplorer);
    void hideUpdateNotifications(bool hidden);
    void websocketStatusChanged(bool enabled);
    void proxySettingsChanged();
    void updateBalance();
    void offlineMode(bool offline);

public slots:
//    void checkboxExternalLinkWarn();
//    void fiatCurrencySelected(int index);

private slots:
    void onProxySettingsChanged();

private:
    void setupAppearanceTab();
    void setupNetworkTab();
    void setupStorageTab();
    void setupDisplayTab();
    void setupMemoryTab();
    void setupTransactionsTab();
    void setupMiscTab();

    void setupThemeComboBox();
    void setSelection(int index);
    void enableWebsocket(bool enabled);

    QScopedPointer<Ui::SettingsNew> ui;
    QSharedPointer<AppContext> m_ctx;
    Nodes *m_nodes = nullptr;

    QStringList m_themes{"Native", "QDarkStyle", "Breeze/Dark", "Breeze/Light"};
    QStringList m_dateFormats{"yyyy-MM-dd", "MM-dd-yyyy", "dd-MM-yyyy"};
    QStringList m_timeFormats{"hh:mm", "hh:mm ap"};
};

#endif //FEATHER_SETTINGSNEWDIALOG_H
