// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include <QSslSocket>

#include "Application.h"
#include "config-feather.h"
#include "constants.h"
#include "utils/EventFilter.h"
#include "utils/os/Prestium.h"
#include "WindowManager.h"
#include "config.h"
#include <wallet/api/wallet2_api.h>
#include "libwalletqt/Wallet.h"
#include "libwalletqt/WalletManager.h"

#if defined(Q_OS_LINUX) && defined(STACK_TRACE)
#define BOOST_STACKTRACE_LINK
#include <signal.h>
#include <boost/stacktrace.hpp>
#include <fstream>
#endif

#if defined(Q_OS_WIN)
#include <windows.h>
#include <vfw.h>
#endif

#if defined(Q_OS_LINUX) && defined(STATIC)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QComposePlatformInputContextPlugin) // Needed for dead keys on Linux
#endif

#if defined(Q_OS_MAC) && defined(STATIC)
Q_IMPORT_PLUGIN(QDarwinCameraPermissionPlugin)
#endif

#if defined(Q_OS_LINUX) && defined(STACK_TRACE)
void signal_handler(int signum) {
    ::signal(signum, SIG_DFL);
    std::stringstream keyStream;
    keyStream << boost::stacktrace::stacktrace();
    std::cout << keyStream.str();

    // Write stack trace to disk
    if (conf()->get(Config::writeStackTraceToDisk).toBool()) {
        QString crashLogPath{Config::defaultConfigDir().path() + "/crash_report.txt"};
        std::ofstream out(crashLogPath.toStdString());
        out << QString("Version: %1-%2\n").arg(FEATHER_VERSION, FEATHER_COMMIT).toStdString();
        out << QString("OS: %1\n").arg(QSysInfo::prettyProductName()).toStdString();
        out << keyStream.str();
        out.close();
    }

    ::raise(SIGABRT);
}
#endif

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(assets);

#if defined(Q_OS_LINUX) && defined(STACK_TRACE)
    ::signal(SIGSEGV, &signal_handler);
    ::signal(SIGABRT, &signal_handler);
#endif

#if defined(HAS_TOR_BIN)
    Q_INIT_RESOURCE(assets_tor);
#endif

#ifdef _WIN32
if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}
#endif

#if defined(Q_OS_LINUX)
    // PassThrough results in muddy text
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif

    Application app(argc, argv);

    QApplication::setApplicationName("FeatherWallet");
    QApplication::setApplicationVersion(FEATHER_VERSION);

    QCommandLineParser parser;
    parser.setApplicationDescription("Feather - a free Monero desktop wallet");
    QCommandLineOption helpOption = parser.addHelpOption();
    QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption useLocalTorOption("use-local-tor", "Use system wide installed Tor instead of the bundled.");
    parser.addOption(useLocalTorOption);

    QCommandLineOption quietModeOption("quiet", "Limit console output");
    parser.addOption(quietModeOption);

    QCommandLineOption stagenetOption("stagenet", "Stagenet is for development purposes only.");
    parser.addOption(stagenetOption);

    QCommandLineOption testnetOption("testnet", "Testnet is for development purposes only.");
    parser.addOption(testnetOption);

    parser.process(app);

    if (parser.isSet(versionOption) || parser.isSet(helpOption)) {
        return EXIT_SUCCESS;
    }

    if (app.isAlreadyRunning()) {
        qWarning() << "Another instance of Feather is already running";
        return EXIT_SUCCESS;
    }

    bool stagenet = parser.isSet(stagenetOption);
    bool testnet = parser.isSet(testnetOption);
    bool quiet = parser.isSet(quietModeOption);

    // Setup networkType
    if (stagenet)
        constants::networkType = NetworkType::STAGENET;
    else if (testnet)
        constants::networkType = NetworkType::TESTNET;
    else
        constants::networkType = NetworkType::MAINNET;

    QApplication::setQuitOnLastWindowClosed(false);
    QApplication::setDesktopSettingsAware(true); // use system font

    // Setup config directories
    QString configDir = Config::defaultConfigDir().path();
    QString config_dir_tor = QString("%1/%2").arg(configDir, "tor");
    QString config_dir_tordata = QString("%1/%2").arg(configDir, "tor/data");
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
    QString logPath = QString("%1/libwallet.log").arg(configDir);
    Monero::Utils::onStartup();
    Monero::Wallet::init("", "feather", logPath.toStdString(), true);

    bool logLevelFromEnv;
    int logLevel = qEnvironmentVariableIntValue("MONERO_LOG_LEVEL", &logLevelFromEnv);
    if (logLevelFromEnv) {
        conf()->set(Config::logLevel, logLevel);
    } else {
        logLevel = conf()->get(Config::logLevel).toInt();
    }

    if (parser.isSet("quiet") || conf()->get(Config::disableLogging).toBool()) {
        qWarning() << "Logging is disabled";
        WalletManager::instance()->setLogLevel(-1);
    }
    else if (logLevel >= 0 && logLevel <= Monero::WalletManagerFactory::LogLevel_Max) {
        Monero::WalletManagerFactory::setLogLevel(logLevel);
    }

    // Setup wallet directory
    QString walletDir = conf()->get(Config::walletDirectory).toString();
    if (walletDir.isEmpty() || Utils::isPortableMode()) {
        walletDir = Utils::defaultWalletDir();
        conf()->set(Config::walletDirectory, walletDir);
    }
    if (!QDir().mkpath(walletDir))
        qCritical() << "Unable to create dir: " << walletDir;

    // Prestium initial config
    if (conf()->get(Config::firstRun).toBool() && Prestium::detect()) {
        conf()->set(Config::proxy, Config::Proxy::i2p);
        conf()->set(Config::socks5Port, Prestium::i2pPort());
        conf()->set(Config::hideUpdateNotifications, true);
    }

    if (parser.isSet("use-local-tor"))
        conf()->set(Config::useLocalTor, true);

    conf()->set(Config::restartRequired, false);

    if (!quiet) {
        QMap<QString, QString> info;
        info["Qt"] = QT_VERSION_STR;
        info["Feather"] = FEATHER_VERSION;
        if (stagenet) info["Mode"] = "Stagenet";
        else if (testnet) info["Mode"] = "Testnet";
        else info["Mode"] = "Mainnet";
        info["SSL"] = QSslSocket::sslLibraryVersionString();
        info["SSL build"] = QSslSocket::sslLibraryBuildVersionString();
#if defined(TOR_VERSION)
        info["Tor version"] = TOR_VERSION;
#else
        info["Tor version"] = "Not bundled";
#endif
        for (const QString &k: info.keys()) {
            qWarning().nospace().noquote() << QString("%1: %2").arg(k, info[k]);
        }
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
    qRegisterMetaType<TxProofResult>("TxProofResult");
    qRegisterMetaType<QPair<bool, bool>>();

    EventFilter filter;
    app.installEventFilter(&filter);

    auto *pool = QThreadPool::globalInstance();
    if (pool->maxThreadCount() < 8) {
        pool->setMaxThreadCount(8);
    }

    auto wm = windowManager();
    wm->setEventFilter(&filter);

    int exitCode = Application::exec();
    qDebug() << "Application::exec() returned";
    return exitCode;
}
