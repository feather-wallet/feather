// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

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
    explicit WindowManager();

    void wizardOpenWallet();
    void close();
    void closeWindow(MainWindow *window);
    void showWizard(WalletWizard::Page startPage);
    void changeSkin(const QString &skinName);
    void restartApplication(const QString &binaryFilename);

signals:
    void torSettingsChanged();

public slots:
    void onTorSettingsChanged();
    void tryOpenWallet(const QString &path, const QString &password);

private slots:
    void onWalletOpened(Wallet *wallet);
    void onWalletCreated(Wallet *wallet);
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);
    void onInitialNetworkConfigured();
    void onDeviceButtonRequest(quint64 code);
    void onDeviceError(const QString &errorMessage);

private:
    void tryCreateWallet(FeatherSeed seed, const QString &path, const QString &password, const QString &seedOffset);
    void tryCreateWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight);
    void tryCreateWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight);

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
