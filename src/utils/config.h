// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2011 Felix Geyer <debfx@fobos.de>
// SPDX-FileCopyrightText: 2020 KeePassXC Team <team@keepassxc.org>
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_CONFIG_H
#define FEATHER_CONFIG_H

#include <QObject>
#include <QSettings>
#include <QPointer>
#include <QDir>

class Config : public QObject
{
    Q_OBJECT

public:
    Q_DISABLE_COPY(Config)

    enum ConfigKey
    {
        // General
        firstRun,
        warnOnStagenet,
        warnOnTestnet,
        warnOnKiImport,

        homeWidget,
        donateBeg,
        showHistorySyncNotice,

        geometry,
        windowState,
        GUI_HistoryViewState,
        geometryOTSWizard,

        // Wallets
        walletDirectory, // Directory where wallet files are stored
        autoOpenWalletPath,
        recentlyOpenedWallets,

        // Nodes
        nodes,
        nodeSource,
        useOnionNodes,

        // Tabs
        enabledTabs,
        showSearchbar,

        // History
        historyShowFullTxid,

        // Receive
        showUsedAddresses,
        showHiddenAddresses,
        showFullAddresses,
        showChangeAddresses,
        showAddressIndex,
        showAddressLabels,

        // Mining
        miningMode,
        xmrigPath,
        xmrigElevated,
        xmrigThreads,
        xmrigPool,
        xmrigDaemon,
        xmrigNetworkTLS,
        xmrigNetworkTor,
        pools,

        // Settings
        lastSettingsPage,

        // Appearance
        skin,
        amountPrecision,
        dateFormat,
        timeFormat,
        balanceDisplay,
        preferredFiatCurrency,

        // Network -> Proxy
        proxy,
        socks5Host,
        socks5Port,
        socks5User,
        socks5Pass,
        useLocalTor, // Prevents Feather from starting bundled Tor daemon
        torOnlyAllowOnion,
        torPrivacyLevel, // Tor node network traffic strategy
        torManagedPort, // Port for managed Tor daemon
        initSyncThreshold, // Switch to Tor after initial sync threshold blocks

        // Network -> Websocket
        disableWebsocket,

        // Network -> Offline
        offlineMode,

        // Storage -> Logging
        writeStackTraceToDisk,
        disableLogging,
        logLevel,

        // Storage -> Misc
        writeRecentlyOpenedWallets,

        // Display
        hideBalance,
        hideUpdateNotifications,
        hideNotifications,
        warnOnExternalLink,
        inactivityLockEnabled,
        inactivityLockTimeout,
        lockOnMinimize,

        // Transactions
        multiBroadcast,
        offlineTxSigningMethod,
        offlineTxSigningForceKISync,

        // Misc
        blockExplorers,
        blockExplorer,
        redditFrontend,
        localMoneroFrontend,
        bountiesFrontend, // unused
        lastPath,
        
        // UR
        URmsPerFragment,
        URfragmentLength,
        URfountainCode,

        // Camera
        cameraManualExposure,
        cameraExposureTime,

        fiatSymbols,
        cryptoSymbols,

        enabledPlugins,
        restartRequired,

        // Tickers
        tickers,
        tickersShowFiatBalance,
    };

    enum PrivacyLevel {
        allTorExceptNode = 0,
        allTorExceptInitSync,
        allTor
    };

    enum BalanceDisplay {
        totalBalance = 0,
        spendablePlusUnconfirmed,
        spendable
    };

    enum MiningMode {
        Pool = 0,
        Solo
    };

    enum Proxy {
        None = 0,
        Tor,
        i2p,
        socks5
    };

    enum OTSMethod {
        UnifiedResources = 0,
        FileTransfer
    };
    
    ~Config() override;
    QVariant get(ConfigKey key);
    QString getFileName();
    void set(ConfigKey key, const QVariant& value);
    void remove(ConfigKey key);
    void sync();
    void resetToDefaults();

    static QDir defaultConfigDir();

    static Config* instance();

signals:
    void changed(Config::ConfigKey key);

private:
    Config(const QString& fileName, QObject* parent = nullptr);
    explicit Config(QObject* parent);
    void init(const QString& configFileName);

    static QPointer<Config> m_instance;

    QScopedPointer<QSettings> m_settings;
    QHash<QString, QVariant> m_defaults;
};

inline Config* conf()
{
    return Config::instance();
}

#endif //FEATHER_CONFIG_H
