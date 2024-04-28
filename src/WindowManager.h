// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WINDOWMANAGER_H
#define FEATHER_WINDOWMANAGER_H

#include <QObject>

#include "dialog/DocsDialog.h"
#include "dialog/TorInfoDialog.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "MainWindow.h"
#include "utils/nodes.h"
#include "wizard/WalletWizard.h"
#include "Utils.h"

class MainWindow;
class WindowManager : public QObject {
Q_OBJECT

public:
    explicit WindowManager(QObject *parent);
    ~WindowManager() override;

    void setEventFilter(EventFilter *eventFilter);

    void wizardOpenWallet();
    void close();
    void closeWindow(MainWindow *window);
    void showWizard(WalletWizard::Page startPage);
    void restartApplication(const QString &binaryFilename);
    void raise();

    void showSettings(Nodes *nodes, QWidget *parent, bool showProxyTab = false);

    void showDocs(QObject *parent, const QString &doc = "", bool modal = false);
    void setDocsHighlight(const QString &highlight);

    void notify(const QString &title, const QString &message, int duration);

    EventFilter *eventFilter;

    static WindowManager* instance();

signals:
    void proxySettingsChanged();
    void websocketStatusChanged(bool enabled);
    void updateBalance();
    void preferredFiatCurrencyChanged();
    void offlineMode(bool offline);
    void pluginConfigured(const QString &id);
    void manualFeeSelectionEnabled(bool enabled);
    void subtractFeeFromAmountEnabled(bool enabled);

public slots:
    void onProxySettingsChanged();
    void onWebsocketStatusChanged(bool enabled);
    void tryOpenWallet(const QString &path, const QString &password);

private slots:
    void onWalletOpened(Wallet *wallet);
    void onWalletCreated(Wallet *wallet);
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);
    void onInitialNetworkConfigured();
    void onDeviceButtonRequest(quint64 code);
    void onDeviceButtonPressed();
    void onDeviceError(const QString &errorMessage);
    void onWalletPassphraseNeeded(bool on_device);
    void onChangeTheme(const QString &themeName);

private:
    void tryCreateWallet(Seed seed, const QString &path, const QString &password, const QString &seedLanguage, const QString &seedOffset, const QString &subaddressLookahead, bool newWallet);
    void tryCreateWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight, const QString &subaddressLookahead);
    void tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, const QString &subaddressLookahead);

    bool autoOpenWallet();

    void initWizard();
    WalletWizard* createWizard(WalletWizard::Page startPage);

    void handleWalletError(const Utils::Message &message);
    void handleDeviceError(const QString &error, Utils::Message &msg);
    void displayWalletErrorMessage(const QString &message);

    void initSkins();
    QString loadStylesheet(const QString &resource);
    void patchMacStylesheet();

    void buildTrayMenu();
    void startupWarning();
    void showWarningMessageBox(const QString &title, const QString &message);
    void showCrashLogs();

    void quitAfterLastWindow();

    static QPointer<WindowManager> m_instance;

    QVector<MainWindow*> m_windows;

    WalletManager *m_walletManager;
    WalletWizard *m_wizard = nullptr;
    SplashDialog *m_splashDialog = nullptr;
    DocsDialog *m_docsDialog = nullptr;

    QSystemTrayIcon *m_tray = nullptr;

    QMap<QString, QString> m_skins;

    bool m_openWalletTriedOnce = false;
    bool m_openingWallet = false;
    bool m_initialNetworkConfigured = false;

    QThread *m_cleanupThread;
};

inline WindowManager* windowManager()
{
    return WindowManager::instance();
}

#endif //FEATHER_WINDOWMANAGER_H
