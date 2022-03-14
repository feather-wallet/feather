// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2011 Felix Geyer <debfx@fobos.de>
// SPDX-FileCopyrightText: 2020 KeePassXC Team <team@keepassxc.org>
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

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
        logLevel,

        homeWidget,
        donateBeg,
        showHistorySyncNotice,

        geometry,
        windowState,
        GUI_HistoryViewState,

        // Wallets
        walletDirectory, // Directory where wallet files are stored
        autoOpenWalletPath,
        recentlyOpenedWallets,

        // Nodes
        nodes,
        nodeSource,
        useOnionNodes,

        // Tabs
        showTabHome,
        showTabCoins,
        showTabExchange,
        showTabCalc,
        showTabXMRig,
        showSearchbar,

        // Mining
        miningMode,
        xmrigPath,
        xmrigElevated,
        xmrigThreads,
        xmrigPool,
        xmrigNetworkTLS,
        xmrigNetworkTor,
        pools,

        // Settings
        lastSettingsPage,
        preferredFiatCurrency,
        skin,
        amountPrecision,
        dateFormat,
        timeFormat,
        balanceDisplay,
        inactivityLockEnabled,
        inactivityLockTimeout,
        disableWebsocket,
        offlineMode,

        multiBroadcast,
        warnOnExternalLink,
        hideBalance,
        disableLogging,

        blockExplorer,
        redditFrontend,
        localMoneroFrontend,

        fiatSymbols,
        cryptoSymbols,

        // Tor
        torPrivacyLevel,
        socks5Host,
        socks5Port,
        socks5User,
        socks5Pass,
        useLocalTor, // Prevents Feather from starting bundled Tor daemon
        initSyncThreshold
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

    ~Config() override;
    QVariant get(ConfigKey key);
    QString getFileName();
    void set(ConfigKey key, const QVariant& value);
    void remove(ConfigKey key);
    void sync();
    void resetToDefaults();

    static QDir defaultConfigDir();
    static QDir defaultPortableConfigDir();

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

inline Config* config()
{
    return Config::instance();
}

#endif //FEATHER_CONFIG_H
