// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <utility>

#include "appcontext.h"
#include "components.h"
#include "calcwindow.h"
#include "settings.h"

#include "dialog/aboutdialog.h"
#include "dialog/signverifydialog.h"
#include "dialog/verifyproofdialog.h"
#include "dialog/seeddialog.h"
#include "dialog/passwordchangedialog.h"
#include "dialog/keysdialog.h"
#include "dialog/aboutdialog.h"
#include "dialog/restoredialog.h"
#include "dialog/splashdialog.h"
#include "libwalletqt/Wallet.h"
#include "model/SubaddressModel.h"
#include "model/SubaddressProxyModel.h"
#include "model/TransactionHistoryModel.h"
#include "model/CoinsModel.h"
#include "model/CoinsProxyModel.h"
#include "utils/networking.h"
#include "utils/config.h"
#include "widgets/ccswidget.h"
#include "widgets/redditwidget.h"
#include "widgets/tickerwidget.h"
#include "wizard/WalletWizard.h"

#ifdef HAS_LOCALMONERO
#include "widgets/LocalMoneroWidget.h"
#endif

#ifdef HAS_XMRIG
#include "widgets/xmrigwidget.h"
#endif

#ifdef Q_OS_MAC
#include "src/kdmactouchbar.h"
#endif

namespace Ui {
    class MainWindow;
}

struct ToggleTab {
    ToggleTab(QWidget *tab, QString name, QString description, QAction *menuAction, Config::ConfigKey configKey) :
            tab(tab), key(std::move(name)), name(std::move(description)), menuAction(menuAction), configKey(configKey){}
    QWidget *tab;
    QString key;
    QString name;
    QAction *menuAction;
    Config::ConfigKey configKey;
};

class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(AppContext *ctx, QWidget *parent = nullptr);
    static MainWindow *getInstance();
    static AppContext *getContext();
    ~MainWindow() override;

    enum Tabs {
        HOME = 0,
        HISTORY,
        SEND,
        RECEIVE,
        COINS,
        CALC,
        EXCHANGES,
        XMRIG
    };

    enum TabsHome {
        CCS,
        REDDIT
    };

public slots:
    void showWizard(WalletWizard::Page startPage);
    void menuNewRestoreClicked();
    void menuQuitClicked();
    void menuSettingsClicked();
    void menuAboutClicked();
    void menuSignVerifyClicked();
    void menuVerifyTxProof();
    void showWalletInfoDialog();
    void showSeedDialog();
    void showConnectionStatusDialog();
    void showPasswordDialog();
    void showKeysDialog();
    void showViewOnlyDialog();
    void donateButtonClicked();
    void showCalcWindow();
    void payToMany();
    void showWalletCacheDebugDialog();
    void showSendTab();
    void showHistoryTab();
    void showSendScreen(const CCSEntry &entry);
    void skinChanged(const QString &skinName);
    void menuTorClicked();
    void onBlockchainSync(int height, int target);
    void onRefreshSync(int height, int target);
    void onWalletOpenedError(const QString &err);
    void onWalletCreatedError(const QString &err);
    void menuWalletCloseClicked();
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);
    void onDeviceButtonRequest(quint64 code);
    void onViewOnBlockExplorer(const QString &txid);
    void onResendTransaction(const QString &txid);
    void importContacts();
    void showRestoreHeightDialog();
    void importTransaction();
    void onDeviceError(const QString &error);
    void menuHwDeviceClicked();
    void onUpdatesAvailable(const QJsonObject &updates);

    // offline tx signing
    void exportKeyImages();
    void importKeyImages();
    void exportOutputs();
    void importOutputs();
    void loadUnsignedTx();
    void loadUnsignedTxFromClipboard();
    void loadSignedTx();
    void loadSignedTxFromText();

    // libwalletqt
    void onBalanceUpdated(quint64 balance, quint64 spendable);
    void onSynchronized();
    void onWalletOpened();
    void onWalletClosed(WalletWizard::Page page = WalletWizard::Page_Menu);
    void onConnectionStatusChanged(int status);
    void onCreateTransactionError(const QString &message);
    void onCreateTransactionSuccess(PendingTransaction *tx, const QVector<QString> &address);
    void onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);

signals:
    void closed();

private slots:
    void onInitialNetworkConfigured();
    void onCheckUpdatesComplete(const QString &version, const QString &binaryFilename, const QString &hash, const QString &signer);
    void onShowUpdateCheck(const QString &version, const QString &binaryFilename, const QString &hash, const QString &signer);
    void onRestartApplication(const QString &binaryFilename);
    void onSignedHashesReceived(QNetworkReply *reply, const QString &platformTag, const QString &version);
    void onShowDonationNag();
    void onInitiateTransaction();
    void onEndTransaction();
    void onCustomRestoreHeightSet(int height);
    void onWalletAboutToClose();

    // Menu
    void onExportHistoryCSV(bool checked);
    void onExportContactsCSV(bool checked);
    void onCreateDesktopEntry(bool checked);
    void onReportBug(bool checked);

private:
    void initSkins();
    void initStatusBar();
    void initWidgets();
    void initMenu();
    void initTray();
    void initHome();
    void initTouchBar();
    void initWalletContext();
    void initWizard();
    void startupWarning();
    bool autoOpenWallet();

    static MainWindow * pMainWindow;
    void closeEvent(QCloseEvent *event) override;
    void cleanupBeforeClose();
    QString loadStylesheet(const QString &resource);
    void saveGeo();
    void restoreGeo();
    void showDebugInfo();
    void showNodeExhaustedMessage();
    void showWSNodeExhaustedMessage();
    void createUnsignedTxDialog(UnsignedTransaction *tx);
    void touchbarShowWizard();
    void touchbarShowWallet();
    void updatePasswordIcon();
    void updateNetStats();
    void rescanSpent();
    void setStatusText(const QString &text, bool override = false, int timeout = 1000);
    void showBalanceDialog();
    QString statusDots();
    void bringToFront();
    QString getPlatformTag();
    void displayWalletErrorMsg(const QString &err);
    QString getHardwareDevice();
    void setTitle(bool mining);

    WalletWizard *createWizard(WalletWizard::Page startPage);

    Ui::MainWindow *ui;
    AppContext *m_ctx;

    Settings *m_windowSettings = nullptr;
    CalcWindow *m_windowCalc = nullptr;
    RestoreDialog *m_restoreDialog = nullptr;
    XMRigWidget *m_xmrig = nullptr;
    SplashDialog *m_splashDialog = nullptr;

#ifdef HAS_LOCALMONERO
    LocalMoneroWidget *m_localMoneroWidget = nullptr;
#endif

    QSystemTrayIcon *m_trayIcon;
    QMenu m_trayMenu;
    QAction *m_trayActionCalc;
    QAction *m_trayActionExit;
    QAction *m_trayActionSend;
    QAction *m_trayActionHistory;

    QList<TickerWidget*> m_tickerWidgets;
    TickerWidget *m_balanceWidget;

    // lower status bar
    QPushButton *m_statusUpdateAvailable;
    ClickableLabel *m_statusLabelBalance;
    QLabel *m_statusLabelStatus;
    QLabel *m_statusLabelNetStats;
    StatusBarButton *m_statusBtnConnectionStatusIndicator;
    StatusBarButton *m_statusBtnPassword;
    StatusBarButton *m_statusBtnPreferences;
    StatusBarButton *m_statusBtnSeed;
    StatusBarButton *m_statusBtnTor;
    StatusBarButton *m_statusBtnHwDevice;

#ifdef Q_OS_MAC
    QAction *m_touchbarActionWelcome;
    KDMacTouchBar *m_touchbar;
    QList<QAction *> m_touchbarWalletItems;
    QList<QAction *> m_touchbarWizardItems;
#endif

    QSignalMapper *m_tabShowHideSignalMapper;
    QMap<QString, ToggleTab*> m_tabShowHideMapper;
    WalletWizard *m_wizard = nullptr;

    QMap<QString, QString> m_skins;

    QTimer m_updateBytes;

    QString m_statusText;
    int m_statusDots;
    bool m_constructingTransaction = false;
    bool m_statusOverrideActive = false;
    bool m_showDeviceError = false;
    QTimer m_txTimer;

private slots:
    void menuToggleTabVisible(const QString &key);
};

#endif // MAINWINDOW_H
