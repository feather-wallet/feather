// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QResource>
#include <QApplication>
#include <QtCore>
#include <QtGui>

#include "cli.h"
#include "config-feather.h"
#include "constants.h"
#include "MainWindow.h"
#include "WindowManager.h"

#if defined(Q_OS_WIN)
#include <windows.h>
#include <vfw.h>
#endif

#if defined(Q_OS_LINUX) && defined(STATIC)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(assets);

#if defined(Q_OS_MAC) && defined(HAS_TOR_BIN)
    Q_INIT_RESOURCE(assets_tor_macos);
#elif defined(HAS_TOR_BIN)
    Q_INIT_RESOURCE(assets_tor);
#endif

#ifdef _WIN32
if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}
#endif

    QStringList argv_;
    for(int i = 0; i != argc; i++){
        argv_ << QString::fromStdString(argv[i]);
    }

    QCommandLineParser parser;
    parser.setApplicationDescription("feather");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption useLocalTorOption(QStringList() << "use-local-tor", "Use system wide installed Tor instead of the bundled.");
    parser.addOption(useLocalTorOption);

    QCommandLineOption torHostOption(QStringList() << "tor-host", "Address of running Tor instance.", "torHost");
    parser.addOption(torHostOption);

    QCommandLineOption torPortOption(QStringList() << "tor-port", "Port of running Tor instance.", "torPort");
    parser.addOption(torPortOption);

    QCommandLineOption quietModeOption(QStringList() << "quiet", "Limit console output");
    parser.addOption(quietModeOption);

    QCommandLineOption stagenetOption(QStringList() << "stagenet", "Stagenet is for development purposes only.");
    parser.addOption(stagenetOption);

    QCommandLineOption testnetOption(QStringList() << "testnet", "Testnet is for development purposes only.");
    parser.addOption(testnetOption);

    QCommandLineOption walletPathOption(QStringList() << "wallet-file", "Path to wallet keys file.", "file");
    parser.addOption(walletPathOption);

    QCommandLineOption walletPasswordOption(QStringList() << "password", "Wallet password (escape/quote as needed)", "password");
    parser.addOption(walletPasswordOption);

    QCommandLineOption daemonAddressOption(QStringList() << "daemon-address", "Daemon address (IPv4:port)", "daemonAddress");
    parser.addOption(daemonAddressOption);

    QCommandLineOption exportContactsOption(QStringList() << "export-contacts", "Output wallet contacts as CSV to specified path.", "file");
    parser.addOption(exportContactsOption);

    QCommandLineOption exportTxHistoryOption(QStringList() << "export-txhistory", "Output wallet transaction history as CSV to specified path.", "file");
    parser.addOption(exportTxHistoryOption);

    QCommandLineOption bruteforcePasswordOption(QStringList() << "bruteforce-password", "Bruteforce wallet password", "file");
    parser.addOption(bruteforcePasswordOption);

    QCommandLineOption bruteforceCharsOption(QStringList() << "bruteforce-chars", "Chars used to bruteforce password", "string");
    parser.addOption(bruteforceCharsOption);

    QCommandLineOption bruteforceDictionairy(QStringList() << "bruteforce-dict", "Bruteforce dictionairy", "file");
    parser.addOption(bruteforceDictionairy);

    bool parsed = parser.parse(argv_);
    if (!parsed) {
        qCritical() << parser.errorText();
        exit(1);
    }

    const QStringList args = parser.positionalArguments();
    bool stagenet = parser.isSet(stagenetOption);
    bool testnet = parser.isSet(testnetOption);
    bool quiet = parser.isSet(quietModeOption);
    bool exportContacts = parser.isSet(exportContactsOption);
    bool exportTxHistory = parser.isSet(exportTxHistoryOption);
    bool bruteforcePassword = parser.isSet(bruteforcePasswordOption);
    bool cliMode = exportContacts || exportTxHistory || bruteforcePassword;

    // Setup networkType
    if (stagenet)
        constants::networkType = NetworkType::STAGENET;
    else if (testnet)
        constants::networkType = NetworkType::TESTNET;
    else
        constants::networkType = NetworkType::MAINNET;

    // Setup QApplication
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_DisableWindowContextHelpButton);
    QApplication::setDesktopSettingsAware(true); // use system font
    QApplication::setApplicationVersion(FEATHER_VERSION);

    QApplication app(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setApplicationName("FeatherWallet");

    // Setup config directories
    QString configDir = Config::defaultConfigDir().path();
    QString config_dir_tor = QString("%1/%2").arg(configDir).arg("tor");
    QString config_dir_tordata = QString("%1/%2").arg(configDir).arg("tor/data");
    QStringList createDirs({configDir, config_dir_tor, config_dir_tordata});
    for (const auto &d: createDirs) {
        if (!Utils::dirExists(d)) {
            qDebug() << QString("Creating directory: %1").arg(d);
            if (!QDir().mkpath(d)) {
                qCritical() << "Could not create directory " << d;
            }
        }
    }

    // Setup logging
    QString logPath = QString("%1/daemon.log").arg(configDir);
    Monero::Utils::onStartup();
    Monero::Wallet::init("", "feather", logPath.toStdString(), true);

    bool logLevelFromEnv;
    int logLevel = qEnvironmentVariableIntValue("MONERO_LOG_LEVEL", &logLevelFromEnv);

    if (parser.isSet("quiet"))
        WalletManager::instance()->setLogLevel(-1);
    else if (logLevelFromEnv && logLevel >= 0 && logLevel <= Monero::WalletManagerFactory::LogLevel_Max)
        Monero::WalletManagerFactory::setLogLevel(logLevel);

    // Setup wallet directory
    QString walletDir = config()->get(Config::walletDirectory).toString();
    if (walletDir.isEmpty()) {
        walletDir = Utils::defaultWalletDir();
        config()->set(Config::walletDirectory, walletDir);
    }
    if (!QDir().mkpath(walletDir))
        qCritical() << "Unable to create dir: " << walletDir;

    // Setup Tor config
    if (parser.isSet("tor-host"))
        config()->set(Config::socks5Host, parser.value("tor-host"));
    if (parser.isSet("tor-port"))
        config()->set(Config::socks5Port, parser.value("tor-port"));
    if (parser.isSet("use-local-tor"))
        config()->set(Config::useLocalTor, true);

    if (cliMode) {
        CLI::Mode mode = [&]{
            if (exportContacts)
                return CLI::Mode::ExportContacts;
            if (exportTxHistory)
                return CLI::Mode::ExportTxHistory;
            if (bruteforcePassword)
                return CLI::Mode::BruteforcePassword;
        }();

        CLI cli{mode, &parser, &app};
        return QCoreApplication::exec();
    }

    parser.process(app); // Parse again for --help and --version

    if (!quiet) {
        QMap<QString, QString> info;
        info["Qt"] = QT_VERSION_STR;
        info["Feather"] = FEATHER_VERSION;
        if (stagenet) info["Mode"] = "Stagenet";
        else if (testnet) info["Mode"] = "Testnet";
        else info["Mode"] = "Mainnet";
        info["SSL"] = QSslSocket::sslLibraryVersionString();
        info["SSL build"] = QSslSocket::sslLibraryBuildVersionString();
        for (const auto &k: info.keys())
            qWarning().nospace().noquote() << QString("%1: %2").arg(k).arg(info[k]);
    }

#if defined(Q_OS_MAC)
    // For some odd reason, if we don't do this, QPushButton's
    // need to be clicked *twice* in order to fire ?!
    QFont fontDef = QApplication::font();
    fontDef.setPointSize(fontDef.pointSize() + 1);
    QApplication::setFont(fontDef);
#endif

    qInstallMessageHandler(Utils::applicationLogHandler);
    qRegisterMetaType<QVector<QString>>();

    WindowManager windowManager;

    return QApplication::exec();
}
