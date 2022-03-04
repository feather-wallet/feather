// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2011 Felix Geyer <debfx@fobos.de>
// SPDX-FileCopyrightText: 2020 KeePassXC Team <team@keepassxc.org>
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "config.h"
#include "utils/Utils.h"
#include "utils/os/tails.h"

#define QS QStringLiteral

struct ConfigDirective
{
    QString name;
    QVariant defaultValue;
};

static const QHash<Config::ConfigKey, ConfigDirective> configStrings = {
        // General
        {Config::firstRun, {QS("firstRun"), true}},
        {Config::warnOnStagenet,{QS("warnOnStagenet"), true}},
        {Config::warnOnTestnet,{QS("warnOnTestnet"), true}},
        {Config::logLevel,{QS("logLevel"), 0}},

        {Config::homeWidget,{QS("homeWidget"), "ccs"}},
        {Config::donateBeg,{QS("donateBeg"), 1}},
        {Config::showHistorySyncNotice, {QS("showHistorySyncNotice"), true}},

        {Config::geometry, {QS("geometry"), {}}},
        {Config::windowState, {QS("windowState"), {}}},
        {Config::GUI_HistoryViewState, {QS("GUI_HistoryViewState"), {}}},

        // Wallets
        {Config::walletDirectory,{QS("walletDirectory"), ""}},
        {Config::autoOpenWalletPath,{QS("autoOpenWalletPath"), ""}},
        {Config::recentlyOpenedWallets, {QS("recentlyOpenedWallets"), {}}},

        // Nodes
        {Config::nodes,{QS("nodes"), "{}"}},
        {Config::nodeSource,{QS("nodeSource"), 0}},
        {Config::useOnionNodes,{QS("useOnionNodes"), false}},

        // Tabs
        {Config::showTabHome,{QS("showTabHome"), true}},
        {Config::showTabCoins,{QS("showTabCoins"), false}},
        {Config::showTabExchange, {QS("showTabExchange"), false}},
        {Config::showTabXMRig,{QS("showTabXMRig"), false}},
        {Config::showTabCalc,{QS("showTabCalc"), true}},
        {Config::showSearchbar,{QS("showSearchbar"), true}},

        // Mining
        {Config::miningMode,{QS("miningMode"), Config::MiningMode::Pool}},
        {Config::xmrigPath,{QS("xmrigPath"), ""}},
        {Config::xmrigElevated,{QS("xmrigElevated"), false}},
        {Config::xmrigThreads,{QS("xmrigThreads"), 1}},
        {Config::xmrigPool,{QS("xmrigPool"), "pool.xmr.pt:9000"}},
        {Config::xmrigNetworkTLS,{QS("xmrigNetworkTLS"), true}},
        {Config::xmrigNetworkTor,{QS("xmrigNetworkTor"), false}},
        {Config::pools,{QS("pools"), {}}},

        // Settings
        {Config::preferredFiatCurrency,{QS("preferredFiatCurrency"), "USD"}},
        {Config::skin,{QS("skin"), "light"}},
        {Config::amountPrecision, {QS("amountPrecision"), 12}},
        {Config::dateFormat, {QS("dateFormat"), "yyyy-MM-dd"}},
        {Config::timeFormat, {QS("timeFormat"), "HH:mm"}},
        {Config::balanceDisplay, {QS("balanceDisplay"), Config::BalanceDisplay::spendablePlusUnconfirmed}},
        {Config::inactivityLockEnabled, {QS("inactivityLockEnabled"), false}},
        {Config::inactivityLockTimeout, {QS("inactivityLockTimeout"), 10}},
        {Config::disableWebsocket, {QS("disableWebsocket"), false}},

        {Config::multiBroadcast, {QS("multiBroadcast"), true}},
        {Config::warnOnExternalLink,{QS("warnOnExternalLink"), true}},
        {Config::hideBalance, {QS("hideBalance"), false}},
        {Config::disableLogging, {QS("disableLogging"), false}},

        {Config::blockExplorer,{QS("blockExplorer"), "exploremonero.com"}},
        {Config::redditFrontend, {QS("redditFrontend"), "old.reddit.com"}},
        {Config::localMoneroFrontend, {QS("localMoneroFrontend"), "https://localmonero.co"}},

        {Config::fiatSymbols, {QS("fiatSymbols"), QStringList{"USD", "EUR", "GBP", "CAD", "AUD", "RUB"}}},
        {Config::cryptoSymbols, {QS("cryptoSymbols"), QStringList{"BTC", "ETH", "LTC", "XMR", "ZEC"}}},

        // Tor
        {Config::torPrivacyLevel, {QS("torPrivacyLevel"), 1}},
        {Config::socks5Host, {QS("socks5Host"), "127.0.0.1"}},
        {Config::socks5Port, {QS("socks5Port"), "9050"}},
        {Config::socks5User, {QS("socks5User"), ""}}, // Unused
        {Config::socks5Pass, {QS("socks5Pass"), ""}}, // Unused
        {Config::useLocalTor, {QS("useLocalTor"), false}},
        {Config::initSyncThreshold, {QS("initSyncThreshold"), 360}}
};


QPointer<Config> Config::m_instance(nullptr);

QVariant Config::get(ConfigKey key)
{
    auto cfg = configStrings[key];
    auto defaultValue = configStrings[key].defaultValue;

    return m_settings->value(cfg.name, defaultValue);
}

QString Config::getFileName()
{
    return m_settings->fileName();
}

void Config::set(ConfigKey key, const QVariant& value)
{
    if (get(key) == value) {
        return;
    }

    auto cfg = configStrings[key];
    m_settings->setValue(cfg.name, value);

    this->sync();
    emit changed(key);
}

void Config::remove(ConfigKey key)
{
    auto cfg = configStrings[key];
    m_settings->remove(cfg.name);

    emit changed(key);
}

/**
 * Sync configuration with persistent storage.
 *
 * Usually, you don't need to call this method manually, but if you are writing
 * configurations after an emitted \link QCoreApplication::aboutToQuit() signal,
 * use it to guarantee your config values are persisted.
 */
void Config::sync()
{
    m_settings->sync();
}

void Config::resetToDefaults()
{
    m_settings->clear();
}

Config::Config(const QString& fileName, QObject* parent)
        : QObject(parent)
{
    init(fileName);
}

Config::Config(QObject* parent)
    : QObject(parent)
{
    QDir configDir = Config::defaultConfigDir();

    if (!QDir().mkpath(configDir.path())) {
        qWarning() << "Unable to create config path: " << configDir.path();
    }

    QString configPath = configDir.filePath("settings.json");

    init(QDir::toNativeSeparators(configPath));
}

QDir Config::defaultConfigDir() {
    QString portablePath = QCoreApplication::applicationDirPath().append("/%1");
    if (QFile::exists(portablePath.arg(".portable")) || QFile::exists(portablePath.arg(".portable.txt"))) {
        return portablePath.arg("feather_data");
    }

    if (TailsOS::detect()) {
        QString path = []{
            QString appImagePath = qgetenv("APPIMAGE");
            if (appImagePath.isEmpty()) {
                qDebug() << "Not an appimage, using currentPath()";
                return QDir::currentPath() + "/.feather/.config/feather";
            }

            QFileInfo appImageDir(appImagePath);
            return appImageDir.absoluteDir().path() + "/.feather/.config/feather";
        }();

        return QDir(path);
    }

#if defined(Q_OS_WIN)
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#elif defined(Q_OS_MACOS)
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#else
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/feather");
#endif
}

QDir Config::defaultPortableConfigDir() {
    return QDir(QCoreApplication::applicationDirPath() + "/feather_data");
}

Config::~Config()
{
}

void Config::init(const QString& configFileName)
{
    const QSettings::Format jsonFormat = QSettings::registerFormat("json", Utils::readJsonFile, Utils::writeJsonFile);
    QSettings::setDefaultFormat(jsonFormat);
    m_settings.reset(new QSettings(configFileName, jsonFormat));

    connect(qApp, &QCoreApplication::aboutToQuit, this, &Config::sync);
}

Config* Config::instance()
{
    if (!m_instance) {
        m_instance = new Config(qApp);
    }

    return m_instance;
}