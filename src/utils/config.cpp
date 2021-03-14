// SPDX-License-Identifier: BSD-3-Clause
// Copyright (C) 2020 KeePassXC Team <team@keepassxc.org>
// Copyright (C) 2011 Felix Geyer <debfx@fobos.de>
// Copyright (c) 2020-2021, The Monero Project.

#include "config.h"
#include "utils/utils.h"
#include "utils/tails.h"

#define QS QStringLiteral

struct ConfigDirective
{
    QString name;
    QVariant defaultValue;
};

static const QHash<Config::ConfigKey, ConfigDirective> configStrings = {
        // General
        {Config::warnOnExternalLink,{QS("warnOnExternalLink"), true}},
        {Config::checkForAppUpdates,{QS("checkForAppUpdates"), true}},
        {Config::warnOnStagenet,{QS("warnOnStagenet"), true}},
        {Config::warnOnTestnet,{QS("warnOnTestnet"), true}},
        {Config::warnOnAlpha,{QS("warnOnAlpha"), true}},
        {Config::homeWidget,{QS("homeWidget"), "ccs"}},
        {Config::donateBeg,{QS("donateBeg"), 1}},
        {Config::skin,{QS("skin"), "light"}},
        {Config::preferredFiatCurrency,{QS("preferredFiatCurrency"), "USD"}},
        {Config::blockExplorer,{QS("blockExplorer"), "exploremonero.com"}},
        {Config::walletDirectory,{QS("walletDirectory"), ""}},
        {Config::autoOpenWalletPath,{QS("autoOpenWalletPath"), ""}},
        {Config::walletPath,{QS("walletPath"), ""}},
        {Config::xmrigPath,{QS("xmrigPath"), ""}},
        {Config::xmrigPool,{QS("xmrigPool"), "pool.xmr.pt:9000"}},
        {Config::nodes,{QS("nodes"), "{}"}},
        {Config::websocketEnabled,{QS("websocketEnabled"), true}},
        {Config::nodeSource,{QS("nodeSource"), 0}},
        {Config::useOnionNodes,{QS("useOnionNodes"), false}},
        {Config::showTabHome,{QS("showTabHome"), true}},
        {Config::showTabCoins,{QS("showTabCoins"), false}},
        {Config::showTabExchange, {QS("showTabExchange"), true}},
        {Config::showTabXMRig,{QS("showTabXMRig"), false}},
        {Config::showTabCalc,{QS("showTabCalc"), true}},
        {Config::geometry, {QS("geometry"), {}}},
        {Config::windowState, {QS("windowState"), {}}},
        {Config::firstRun,{QS("firstRun"), false}},
        {Config::hideBalance, {QS("hideBalance"), false}},
        {Config::redditFrontend, {QS("redditFrontend"), "old.reddit.com"}},
        {Config::showHistorySyncNotice, {QS("showHistorySyncNotice"), true}},
        {Config::GUI_HistoryViewState, {QS("GUI_HistoryViewState"), {}}},
        {Config::amountPrecision, {QS("amountPrecision"), 4}},
        {Config::dateFormat, {QS("dateFormat"), "yyyy-MM-dd"}},
        {Config::timeFormat, {QS("timeFormat"), "HH:mm"}}
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

    QString portablePath = QCoreApplication::applicationDirPath().append("/%1");
    if (QFile::exists(portablePath.arg(".portable"))) {
        init(portablePath.arg("feather_data/settings.json"));
        return;
    }

    bool isTails = TailsOS::detect();
    if (isTails) { // #if defined(PORTABLE)
        QString appImagePath = qgetenv("APPIMAGE");
        QFileInfo appImageDir(appImagePath);

        QDir portablePath(appImageDir.absoluteDir().path() + "/feather_data");
        if (portablePath.mkpath(".")) {
            configDir = portablePath;
        } else {
            qCritical() << "Unable to create portable directory: " << portablePath.path();
        }
    }

    QString configPath = configDir.filePath("settings.json");

    init(QDir::toNativeSeparators(configPath));
}

QDir Config::defaultConfigDir() {
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