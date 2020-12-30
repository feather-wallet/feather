// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2020 KeePassXC Team <team@keepassxc.org>
// Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_SETTINGS_H
#define FEATHER_SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QPointer>

class Config : public QObject
{
    Q_OBJECT

public:
    Q_DISABLE_COPY(Config)

    enum ConfigKey
    {
        warnOnExternalLink,
        checkForAppUpdates,
        warnOnStagenet,
        warnOnTestnet,
        warnOnAlpha,
        homeWidget,
        donateBeg,
        autoOpenWalletPath,
        skin,
        preferredFiatCurrency,
        blockExplorer,
        walletDirectory,
        walletPath,
        xmrigPath,
        xmrigPool,
        nodes,
        websocketEnabled,
        nodeSource,
        useOnionNodes,
        showTabHome,
        showTabCoins,
        showTabExchange,
        showTabCalc,
        showTabXMRig,
        geometry,
        windowState,
        firstRun,
        hideBalance,
        redditFrontend,
        showHistorySyncNotice
    };

    ~Config() override;
    QVariant get(ConfigKey key);
    QString getFileName();
    void set(ConfigKey key, const QVariant& value);
    void sync();
    void resetToDefaults();

    static Config* instance();

signals:
    void changed(ConfigKey key);

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
