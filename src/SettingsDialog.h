// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SettingsDIALOG_H
#define FEATHER_SettingsDIALOG_H

#include <QDialog>

namespace Ui {
    class Settings;
}

class Nodes;
class Settings : public QDialog
{
Q_OBJECT

public:
    explicit Settings(Nodes *nodes, QWidget *parent = nullptr);
    ~Settings() override;

    void showNetworkProxyTab();

    enum Pages {
        APPEARANCE = 0,
        NETWORK,
        STORAGE,
        DISPLAY,
        MEMORY,
        TRANSACTIONS,
        PLUGINS,
        MISC
    };

signals:
    void preferredFiatCurrencyChanged(QString currency);
    void skinChanged(QString skinName);
    void hideUpdateNotifications(bool hidden);
    void hideTrayIcon(bool hidden);
    void websocketStatusChanged(bool enabled);
    void proxySettingsChanged();
    void updateBalance();
    void offlineMode(bool offline);
    void pluginConfigured(const QString &id);
    void manualFeeSelectionEnabled(bool enabled);
    void subtractFeeFromAmountEnabled(bool enabled);

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
    void setupPluginsTab();
    void setupMiscTab();

    void setupThemeComboBox();
    void setSelection(int index);
    void enableWebsocket(bool enabled);

    QScopedPointer<Ui::Settings> ui;
    Nodes *m_nodes = nullptr;

    QStringList m_themes{"Native", "QDarkStyle", "Breeze/Dark", "Breeze/Light"};
    QStringList m_dateFormats{"yyyy-MM-dd", "MM-dd-yyyy", "dd-MM-yyyy"};
    QStringList m_timeFormats{"hh:mm", "hh:mm ap"};
};

#endif //FEATHER_SettingsDIALOG_H
