// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2020 KeePassXC Team <team@keepassxc.org>
// Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SETTINGS_H
#define FEATHER_SETTINGS_H

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
        firstRun,
        warnOnStagenet,
        warnOnTestnet,
        warnOnAlpha,

        homeWidget,
        donateBeg,
        showHistorySyncNotice,

        geometry,
        windowState,
        GUI_HistoryViewState,

        walletDirectory, // Directory where wallet files are stored
        autoOpenWalletPath,
        recentlyOpenedWallets,

        nodes,
        nodeSource,
        useOnionNodes,

        showTabHome,
        showTabCoins,
        showTabExchange,
        showTabCalc,
        showTabXMRig,

        xmrigPath,
        xmrigPool,

        preferredFiatCurrency,
        skin,
        amountPrecision,
        dateFormat,
        timeFormat,

        multiBroadcast,
        warnOnExternalLink,
        hideBalance,

        blockExplorer,
        redditFrontend,
        localMoneroFrontend,

        torPrivacyLevel,
        socks5Host,
        socks5Port,
        socks5User,
        socks5Pass,
        useLocalTor, // Prevents Feather from starting bundled Tor daemon
    };

    enum PrivacyLevel {
        allTorExceptNode = 0,
        allTorExceptInitSync,
        allTor
    };

    ~Config() override;
    QVariant get(ConfigKey key);
    QString getFileName();
    void set(ConfigKey key, const QVariant& value);
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

#endif //FEATHER_SETTINGS_H
