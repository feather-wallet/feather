// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2011 Felix Geyer <debfx@fobos.de>
// SPDX-FileCopyrightText: 2020 KeePassXC Team <team@keepassxc.org>
// SPDX-FileCopyrightText: The Monero Project

#include "config.h"

#include <QCoreApplication>
#include <QStandardPaths>

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
        {Config::warnOnKiImport,{QS("warnOnKiImport"), true}},
        {Config::logLevel,{QS("logLevel"), 0}},

        {Config::homeWidget,{QS("homeWidget"), "revuo"}},
        {Config::donateBeg,{QS("donateBeg"), 1}},
        {Config::showHistorySyncNotice, {QS("showHistorySyncNotice"), true}},

        {Config::geometry, {QS("geometry"), {}}},
        {Config::windowState, {QS("windowState"), {}}},
        {Config::GUI_HistoryViewState, {QS("GUI_HistoryViewState"), {}}},
        {Config::geometryOTSWizard, {QS("geometryOTSWizard"), {}}},

        // Wallets
        {Config::walletDirectory,{QS("walletDirectory"), ""}},
        {Config::autoOpenWalletPath,{QS("autoOpenWalletPath"), ""}},
        {Config::recentlyOpenedWallets, {QS("recentlyOpenedWallets"), {}}},

        // Nodes
        {Config::nodes,{QS("nodes"), "{}"}},
        {Config::nodeSource,{QS("nodeSource"), 0}},
        {Config::useOnionNodes,{QS("useOnionNodes"), false}},

        // Tabs
        {Config::enabledTabs, {QS("enabledTabs"), QStringList{"Home", "History", "Send", "Receive", "Calc"}}},
        {Config::showSearchbar,{QS("showSearchbar"), true}},

        // History
        {Config::historyShowFullTxid, {QS("historyShowFullTxid"), false}},

        // Receive
        {Config::showUsedAddresses,{QS("showUsedAddresses"), false}},
        {Config::showHiddenAddresses,{QS("showHiddenAddresses"), false}},
        {Config::showFullAddresses, {QS("showFullAddresses"), false}},
        {Config::showChangeAddresses,{QS("showChangeAddresses"), false}},
        {Config::showAddressIndex,{QS("showAddressIndex"), true}},
        {Config::showAddressLabels,{QS("showAddressLabels"), true}},

        // Settings
        {Config::lastSettingsPage, {QS("lastSettingsPage"), 0}},
        {Config::preferredFiatCurrency,{QS("preferredFiatCurrency"), "USD"}},
        {Config::skin,{QS("skin"), "light"}},
        {Config::amountPrecision, {QS("amountPrecision"), 12}},
        {Config::dateFormat, {QS("dateFormat"), "yyyy-MM-dd"}},
        {Config::timeFormat, {QS("timeFormat"), "HH:mm"}},
        {Config::balanceDisplay, {QS("balanceDisplay"), Config::BalanceDisplay::spendablePlusUnconfirmed}},
        {Config::balanceShowFiat, {QS("balanceShowFiat"), false}},
        {Config::inactivityLockEnabled, {QS("inactivityLockEnabled"), false}},
        {Config::inactivityLockTimeout, {QS("inactivityLockTimeout"), 10}},
        {Config::lockOnMinimize, {QS("lockOnMinimize"), false}},
        {Config::showTrayIcon, {QS("showTrayIcon"), true}},
        {Config::minimizeToTray, {QS("minimizeToTray"), false}},
        {Config::disableWebsocket, {QS("disableWebsocket"), false}},
        {Config::offlineMode, {QS("offlineMode"), false}},

        // Transactions
        {Config::multiBroadcast, {QS("multiBroadcast"), true}},
        {Config::offlineTxSigningMethod, {QS("offlineTxSigningMethod"), Config::OTSMethod::UnifiedResources}},
        {Config::offlineTxSigningForceKISync, {QS("offlineTxSigningForceKISync"), false}},
        {Config::manualFeeTierSelection, {QS("manualFeeTierSelection"), false}},
        {Config::subtractFeeFromAmount, {QS("subtractFeeFromAmount"), false}},

        {Config::warnOnExternalLink,{QS("warnOnExternalLink"), true}},
        {Config::hideBalance, {QS("hideBalance"), false}},
        {Config::hideNotifications, {QS("hideNotifications"), false}},
        {Config::hideUpdateNotifications, {QS("hideUpdateNotifications"), false}},
        {Config::disableLogging, {QS("disableLogging"), true}},
        {Config::writeStackTraceToDisk, {QS("writeStackTraceToDisk"), true}},
        {Config::writeRecentlyOpenedWallets, {QS("writeRecentlyOpenedWallets"), true}},

        {Config::blockExplorers, {QS("blockExplorers"), QStringList{"https://xmrchain.net/tx/%txid%",
                                                                    "https://moneroblocks.info/tx/%txid%",
                                                                    "https://blockchair.com/monero/transaction/%txid%",
                                                                    "http://blkchairbknpn73cfjhevhla7rkp4ed5gg2knctvv7it4lioy22defid.onion/monero/transaction/%txid%",
                                                                    "http://127.0.0.1:31312/tx?id=%txid%"}}},
        {Config::blockExplorer,{QS("blockExplorer"), "https://xmrchain.net/tx/%txid%"}},
        {Config::lastPath, {QS("lastPath"), QDir::homePath()}},

        {Config::URmsPerFragment, {QS("URmsPerFragment"), 80}},
        {Config::URfragmentLength, {QS("URfragmentLength"), 150}},
        {Config::URfountainCode, {QS("URfountainCode"), false}},

        {Config::cameraManualExposure, {QS("cameraManualExposure"), false}},
        {Config::cameraExposureTime, {QS("cameraExposureTime"), 10}},

        {Config::fiatSymbols, {QS("fiatSymbols"), QStringList{"USD", "EUR", "GBP", "CAD", "AUD", "RUB"}}},
        {Config::cryptoSymbols, {QS("cryptoSymbols"), QStringList{"BTC", "ETH", "LTC", "XMR", "ZEC"}}},

        // Tor
        {Config::proxy, {QS("proxy"), Config::Proxy::Tor}},
        {Config::torPrivacyLevel, {QS("torPrivacyLevel"), 1}},
        {Config::torOnlyAllowOnion, {QS("torOnlyAllowOnion"), false}},
        {Config::socks5Host, {QS("socks5Host"), "127.0.0.1"}},
        {Config::socks5Port, {QS("socks5Port"), "9050"}},
        {Config::socks5User, {QS("socks5User"), ""}}, // Unused
        {Config::socks5Pass, {QS("socks5Pass"), ""}}, // Unused
        {Config::torManagedPort, {QS("torManagedPort"), "19450"}},
        {Config::useLocalTor, {QS("useLocalTor"), false}},
        {Config::initSyncThreshold, {QS("initSyncThreshold"), 360}},

        {Config::enabledPlugins, {QS("enabledPlugins"), QStringList{"tickers", "revuo", "calc"}}},
        {Config::restartRequired, {QS("restartRequired"), false}},

        {Config::tickers, {QS("tickers"), QStringList{"XMR", "BTC", "XMR/BTC"}}},
        {Config::tickersShowFiatBalance, {QS("tickersShowFiatBalance"), true}},
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
    if (Utils::isPortableMode()) {
        return Utils::portablePath();
    }

    if (TailsOS::detect()) {
#ifdef SELF_CONTAINED
        QString path = []{
            QString appImagePath = qgetenv("APPIMAGE");
            if (appImagePath.isEmpty()) {
                qDebug() << "Not an appimage, using currentPath()";
                if (QDir(QDir::currentPath() + "/.feather").exists()) {
                    return QDir::currentPath() + "/.feather/.config/feather";
                }
                return QDir::currentPath() + "/feather_data";
            }

            QFileInfo appImageDir(appImagePath);
            QString absolutePath = appImageDir.absoluteDir().path();
            if (QDir(absolutePath + "/.feather").exists()) {
                return absolutePath + "/.feather/.config/feather";
            }
            return absolutePath + "/feather_data";
        }();

        return QDir(path);
#else
        if (TailsOS::detectDataPersistence()) {
            return QDir::homePath() + "/Persistent/feather_data";
        }
#endif
    }

#if defined(Q_OS_WIN)
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#elif defined(Q_OS_MACOS)
    return QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
#else
    return QDir(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + "/feather");
#endif
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
