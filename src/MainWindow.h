// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_MAINWINDOW_H
#define FEATHER_MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "components.h"
#include "SettingsDialog.h"

#include "dialog/AboutDialog.h"
#include "dialog/AccountSwitcherDialog.h"
#include "dialog/SignVerifyDialog.h"
#include "dialog/VerifyProofDialog.h"
#include "dialog/SeedDialog.h"
#include "dialog/PasswordChangeDialog.h"
#include "dialog/KeysDialog.h"
#include "dialog/AboutDialog.h"
#include "dialog/SplashDialog.h"
#include "dialog/TxPoolViewerDialog.h"
#include "libwalletqt/Wallet.h"
#include "model/SubaddressModel.h"
#include "model/SubaddressProxyModel.h"
#include "model/TransactionHistoryModel.h"
#include "model/CoinsModel.h"
#include "model/CoinsProxyModel.h"
#include "utils/Networking.h"
#include "utils/config.h"
#include "utils/daemonrpc.h"
#include "utils/EventFilter.h"
#include "widgets/TickerWidget.h"
#include "widgets/WalletUnlockWidget.h"
#include "wizard/WalletWizard.h"

#include "ContactsWidget.h"
#include "HistoryWidget.h"
#include "SendWidget.h"
#include "ReceiveWidget.h"
#include "CoinsWidget.h"

#include "WindowManager.h"
#include "plugins/Plugin.h"

#ifdef CHECK_UPDATES
#include "utils/updater/Updater.h"
#endif

namespace Ui {
    class MainWindow;
}

class ToggleTab : QObject {
Q_OBJECT

public:
    ToggleTab(QWidget *tab, QString name, QString description, QAction *menuAction, QObject *parent = nullptr) :
            QObject(parent), tab(tab), key(std::move(name)), name(std::move(description)), menuAction(menuAction) {}
    QWidget *tab;
    QString key;
    QString name;
    QAction *menuAction;
};

class WindowManager;
class MainWindow : public QMainWindow
{
Q_OBJECT

public:
    explicit MainWindow(WindowManager *windowManager, Wallet *wallet, QWidget *parent = nullptr);
    ~MainWindow() override;

    QString walletName();
    QString walletCachePath();
    QString walletKeysPath();

    enum Stack {
        WALLET = 0,
        LOCKED,
        OFFLINE
    };

    void showOrHide();
    void bringToFront();

public slots:
    void onPreferredFiatCurrencyChanged();
    void onHideUpdateNotifications(bool hidden);

signals:
    void updateIcons();
    void closed();
    void uiSetup();
    void aboutToQuit();

protected:
    void changeEvent(QEvent* event) override;

private slots:
    // TODO: use a consistent naming convention for slots
    // Menu
    void menuOpenClicked();
    void menuNewRestoreClicked();
    void menuQuitClicked();
    void menuSettingsClicked(bool showProxytab = false);
    void menuAboutClicked();
    void menuSignVerifyClicked();
    void menuVerifyTxProof();
    void menuWalletCloseClicked();
    void menuProxySettingsClicked();
    void menuToggleTabVisible(const QString &key);
    void menuClearHistoryClicked();
    void onExportHistoryCSV();
    void onImportHistoryDescriptionsCSV();
    void onExportContactsCSV();
    void onCreateDesktopEntry();
    void onShowDocumentation();
    void onReportBug();
    void onShowSettingsPage(int page);

    // offline tx signing
    void loadSignedTx();
    void loadSignedTxFromText();

    void onTorConnectionStateChanged(bool connected);
    void showUpdateDialog();
    void onInitiateTransaction();
    void onKeysCorrupted();
    void onSelectedInputsChanged(const QStringList &selectedInputs);
    void onTxPoolBacklog(const QVector<quint64> &backlog, quint64 originalFeeLevel, quint64 automaticFeeLevel);

    // libwalletqt
    void updateBalance();
    void onBalanceUpdated(quint64 balance, quint64 spendable);
    void onSyncStatus(quint64 height, quint64 target, bool daemonSync);
    void onWalletOpened();
    void onConnectionStatusChanged(int status);
    void onTransactionCreated(PendingTransaction *tx, const QVector<QString> &address);
    void onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid);

    // Dialogs
    void showWalletInfoDialog();
    void showSeedDialog();
    void showPasswordDialog();
    void showKeysDialog();
    void showViewOnlyDialog();
    void showKeyImageSyncWizard();
    void showWalletCacheDebugDialog();
    void showTxPoolViewerDialog();
    void showAccountSwitcherDialog();
    void showAddressChecker();
    void showURDialog();
    
    void donateButtonClicked();
    void payToMany();
    void showHistoryTab();
    void skinChanged(const QString &skinName);
    void onViewOnBlockExplorer(const QString &txid);
    void onResendTransaction(const QString &txid);
    void importContacts();
    void importTransaction();
    void onDeviceError(const QString &error, quint64 errorCode);
    void onDeviceButtonRequest(quint64 code);
    void onDeviceButtonPressed();
    void onWalletPassphraseNeeded(bool on_device);
    void menuHwDeviceClicked();
    void toggleSearchbar(bool enabled);
    void tryStoreWallet();
    void onWebsocketStatusChanged(bool enabled);
    void showUpdateNotification();
    void onProxySettingsChangedConnect();
    void onProxySettingsChanged();
    void onOfflineMode(bool offline);
    void onManualFeeSelectionEnabled(bool enabled);
    void onSubtractFeeFromAmountEnabled(bool enabled);
    void onMultiBroadcast(const QMap<QString, QString> &txHexMap);

private:
    friend WindowManager;

    void initStatusBar();
    void initPlugins();
    void initWidgets();
    void initMenu();
    void initOffline();
    void initWalletContext();

    void closeEvent(QCloseEvent *event) override;

    void saveGeo();
    void restoreGeo();
    void showDebugInfo();
    void updatePasswordIcon();
    void updateNetStats();
    void rescanSpent();
    void setStatusText(const QString &text, bool override = false, int timeout = 1000);
    void showBalanceDialog();
    QString statusDots();
    QString getHardwareDevice();
    void updateTitle();
    void donationNag();
    void addToRecentlyOpened(QString filename);
    void updateRecentlyOpenedMenu();
    void updateWidgetIcons();
    bool verifyPassword(bool sensitive = true);
    void fillSendTab(const QString &address, const QString &description);
    void userActivity();
    void checkUserActivity();
    void lockWallet();
    void unlockWallet(const QString &password);
    void closeQDialogChildren(QObject *object);
    int findTab(const QString &title);

    QIcon hardwareDevicePairedIcon();
    QIcon hardwareDeviceUnpairedIcon();

    QScopedPointer<Ui::MainWindow> ui;
    WindowManager *m_windowManager;
    Wallet *m_wallet = nullptr;
    Nodes *m_nodes;
    DaemonRpc *m_rpc;

    SplashDialog *m_splashDialog = nullptr;
    AccountSwitcherDialog *m_accountSwitcherDialog = nullptr;
    TxPoolViewerDialog *m_txPoolViewerDialog = nullptr;

    WalletUnlockWidget *m_walletUnlockWidget = nullptr;
    ContactsWidget *m_contactsWidget = nullptr;
    HistoryWidget *m_historyWidget = nullptr;
    SendWidget *m_sendWidget = nullptr;
    ReceiveWidget *m_receiveWidget = nullptr;
    CoinsWidget *m_coinsWidget = nullptr;

    QPointer<QAction> m_clearRecentlyOpenAction;

    // lower status bar
    QPushButton *m_statusUpdateAvailable;
    ClickableLabel *m_statusLabelBalance;
    QLabel *m_statusLabelStatus;
    QLabel *m_statusLabelNetStats;
    StatusBarButton *m_statusAccountSwitcher;
    StatusBarButton *m_statusBtnConnectionStatusIndicator;
    StatusBarButton *m_statusBtnPassword;
    StatusBarButton *m_statusBtnPreferences;
    StatusBarButton *m_statusBtnSeed;
    StatusBarButton *m_statusBtnProxySettings;
    StatusBarButton *m_statusBtnHwDevice;

    QSignalMapper *m_tabShowHideSignalMapper;
    QMap<QString, ToggleTab*> m_tabShowHideMapper;

    QTimer m_updateBytes;
    QTimer m_checkUserActivity;

    QList<Plugin*> m_plugins;

    QString m_statusText;
    int m_statusDots;
    bool m_constructingTransaction = false;
    bool m_statusOverrideActive = false;
    bool m_showDeviceError = false;
    QTimer m_txTimer;

    bool cleanedUp = false;
    bool m_locked = false;
    bool m_criticalWarningShown = false;

    EventFilter *m_eventFilter = nullptr;
    qint64 m_userLastActive = QDateTime::currentSecsSinceEpoch();

#ifdef CHECK_UPDATES
    QSharedPointer<Updater> m_updater = nullptr;
#endif
};

#endif // FEATHER_MAINWINDOW_H
