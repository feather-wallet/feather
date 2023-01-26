// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_WINDOWMANAGER_H
#define FEATHER_WINDOWMANAGER_H

#include <QObject>

#include "dialog/TorInfoDialog.h"
#include "libwalletqt/WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "MainWindow.h"
#include "wizard/WalletWizard.h"

class MainWindow;
class WindowManager : public QObject {
Q_OBJECT

public:
    explicit WindowManager(EventFilter *eventFilter);
    ~WindowManager() override;

    void wizardOpenWallet();
    void close();
    void closeWindow(MainWindow *window);
    void showWizard(WalletWizard::Page startPage);
    void changeSkin(const QString &skinName);
    void restartApplication(const QString &binaryFilename);
    void raise();

    EventFilter *eventFilter;

signals:
    void torSettingsChanged();
    void websocketStatusChanged(bool enabled);

public slots:
    void onTorSettingsChanged();
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

private:
    void tryCreateWallet(Seed seed, const QString &path, const QString &password, const QString &seedLanguage, const QString &seedOffset, const QString &subaddressLookahead);
    void tryCreateWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight, const QString &subaddressLookahead);
    void tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, const QString &subaddressLookahead);

    bool autoOpenWallet();

    void initWizard();
    WalletWizard* createWizard(WalletWizard::Page startPage) const;

    void handleWalletError(const QString &message);
    void displayWalletErrorMessage(const QString &message);

    void initTor();
    void initWS();
    void initSkins();
    QString loadStylesheet(const QString &resource);
    void buildTrayMenu();
    void startupWarning();
    void showWarningMessageBox(const QString &title, const QString &message);
    void showCrashLogs();

    void quitAfterLastWindow();

    QVector<MainWindow*> m_windows;

    WalletManager *m_walletManager;
    WalletWizard *m_wizard = nullptr;
    SplashDialog *m_splashDialog = nullptr;

    QSystemTrayIcon *m_tray;

    QMap<QString, QString> m_skins;

    bool m_openWalletTriedOnce = false;
    bool m_openingWallet = false;
    bool m_initialNetworkConfigured = false;

    QThread *m_cleanupThread;
};


#endif //FEATHER_WINDOWMANAGER_H
