// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "utils/TorManager.h"

#include <QCoreApplication>
#include <QDirIterator>

#include "utils/config.h"
#include "utils/Utils.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"

TorManager::TorManager(QObject *parent)
    : QObject(parent)
    , m_checkConnectionTimer(new QTimer(this))
    , m_process(new QProcess(this))
{
    connect(m_checkConnectionTimer, &QTimer::timeout, this, &TorManager::checkConnection);

    this->torDir = Config::defaultConfigDir().filePath("tor");
#if defined(TOR_INSTALLED)
    // When installed, use directory relative to application path.
    this->torDir = QDir(Utils::applicationPath()).filePath("tor");
#endif
    if (QString(FEATHER_TARGET_TRIPLET) == "arm64-apple-darwin" || QString(FEATHER_TARGET_TRIPLET) == "x86_64-apple-darwin") {
        QString featherBinaryPath = QCoreApplication::applicationDirPath();
        QDir appBinaryDir(featherBinaryPath);
        appBinaryDir.cd("..");
        this->torDir = appBinaryDir.filePath("bin");
    }

    this->torDataPath = Config::defaultConfigDir().filePath("tor/data");

    m_process->setProcessChannelMode(QProcess::MergedChannels);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &TorManager::handleProcessOutput);
    connect(m_process, &QProcess::errorOccurred, this, &TorManager::handleProcessError);
    connect(m_process, &QProcess::stateChanged, this, &TorManager::stateChanged);
}

QPointer<TorManager> TorManager::m_instance(nullptr);

void TorManager::init() {
    m_localTor = !shouldStartTorDaemon();

    auto state = m_process->state();
    if (m_localTor && (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting)) {
        m_process->kill();
        m_started = false;
    }

    featherTorPort = conf()->get(Config::torManagedPort).toString().toUShort();
}

void TorManager::stop() {
    m_process->kill();
    m_started = false;
}

void TorManager::start() {
    m_checkConnectionTimer->start(5000);

    if (m_localTor) {
        this->checkConnection();
        return;
    }

    auto state = m_process->state();
    if (state == QProcess::ProcessState::Running || state == QProcess::ProcessState::Starting) {
        return;
    }

    if (Utils::portOpen(featherTorHost, featherTorPort)) {
        this->setErrorMessage(QString("Unable to start Tor on %1:%2. Port already in use.").arg(featherTorHost, QString::number(featherTorPort)));
        return;
    }

    QFile torFile{this->torPath};
    QString alternativeTorFile = QCoreApplication::applicationDirPath() + "/tor";
    if (!torFile.exists() && QFileInfo(alternativeTorFile).isFile()) {
        this->torPath = alternativeTorFile;
    }

    qDebug() << QString("Start process: %1").arg(this->torPath);

    // Limit restart attempts to prevent infinite loops when Tor is misconfigured.
    // After 4 failed attempts, Tor is permanently disabled for this session.
    // The error is emitted via statusChanged() but there is no UI dialog — the user
    // sees Tor as disconnected in the status bar.
    m_restarts += 1;
    if (m_restarts > 4) {
        this->setErrorMessage("Tor failed to start: maximum retries exceeded");
        return;
    }

    QStringList arguments;

    arguments << "--ignore-missing-torrc";
    arguments << "--SocksPort" << QString("%1:%2").arg(featherTorHost, QString::number(featherTorPort));
    arguments << "--TruncateLogFile" << "1";
    arguments << "--DataDirectory" << this->torDataPath;
    arguments << "--Log" << "notice";
    arguments << "--pidfile" << QDir(this->torDataPath).filePath("tor.pid");

    qDebug() << QString("%1 %2").arg(this->torPath, arguments.join(" "));

    m_process->start(this->torPath, arguments);
    m_started = true;
}

// Checks whether Tor is connected. Called every 5 seconds by m_checkConnectionTimer.
// The check varies by environment (order matters — first match wins):
//   1. Torsocks:  Assume connected (can't probe localhost through torsocks)
//   2. Whonix:    Assume connected (Whonix guarantees all traffic is routed through Tor)
//   3. Tails:     Query systemd for "tails-tor-has-bootstrapped.target"
//   4. Non-Tor:   Not connected (proxy isn't set to Tor)
//   5. Local Tor: Probe user-configured socks5Host:socks5Port (default 127.0.0.1:9050)
//   6. Managed:   Probe featherTorHost:featherTorPort (default 127.0.0.1:19450)
void TorManager::checkConnection() {
    if (Utils::isTorsocks()) {
        this->setConnectionState(true);
    }

    else if (WhonixOS::detect()) {
        this->setConnectionState(true);
    }

    else if (TailsOS::detect()) {
        QStringList args = QStringList() << "--quiet" << "is-active" << "tails-tor-has-bootstrapped.target";
        int code = QProcess::execute("/bin/systemctl", args);

        this->setConnectionState(code == 0);
    }

    else if (conf()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        this->setConnectionState(false);
    }

    else if (m_localTor && !m_alreadyRunning) {
        QString host = conf()->get(Config::socks5Host).toString();
        quint16 port = conf()->get(Config::socks5Port).toString().toUShort();
        this->setConnectionState(Utils::portOpen(host, port));
    }

    else {
        this->setConnectionState(Utils::portOpen(featherTorHost, featherTorPort));
    }
}

void TorManager::setConnectionState(bool connected) {
    this->torConnected = connected;
    emit connectionStateChanged(connected);
}

// Called when the managed Tor process changes state.
// If Tor exits unexpectedly (crash, killed), it is automatically restarted after 1 second
// unless m_stopRetries is set (which happens when the binary fails to start at all).
// The restart goes through start(), which enforces the 4-attempt limit via m_restarts.
void TorManager::stateChanged(QProcess::ProcessState state) {
    if (state == QProcess::ProcessState::Running) {
        this->setErrorMessage("");
        qWarning() << "Tor started, awaiting bootstrap";
    }
    else if (state == QProcess::ProcessState::NotRunning) {
        this->setConnectionState(false);

        if (m_stopRetries)
            return;

        QTimer::singleShot(1000, [=] {
            this->start();
        });
    }
}

void TorManager::handleProcessOutput() {
    QByteArray output = m_process->readAllStandardOutput();
    this->torLogs.append(Utils::barrayToString(output));
    emit logsUpdated();
    if(output.contains(QByteArray("Bootstrapped 100%"))) {
        qDebug() << "Tor OK";
        this->setConnectionState(true);
    }

    qDebug() << output;
}

void TorManager::handleProcessError(QProcess::ProcessError error) {
    if (error == QProcess::ProcessError::Crashed)
        qWarning() << "Tor crashed or killed";
    else if (error == QProcess::ProcessError::FailedToStart) {
        this->setErrorMessage("Tor binary failed to start: " + this->torPath);
        this->m_stopRetries = true;
    }
}

bool TorManager::unpackBins() {
    if (m_unpacked) {
        return true;
    }

    QString torBin = "tor";
#if defined(Q_OS_WIN)
   torBin += ".exe";
#endif

    this->torPath = QDir(this->torDir).filePath(torBin);

#if defined(TOR_INSTALLED)
    // We don't need to unpack if Tor was installed using the installer
    return true;
#endif

    if (QString(FEATHER_TARGET_TRIPLET) == "arm64-apple-darwin" || QString(FEATHER_TARGET_TRIPLET) == "x86_64-apple-darwin") {
        return true;
    }

    SemanticVersion embeddedVersion = SemanticVersion::fromString(QString(TOR_VERSION));
    SemanticVersion filesystemVersion = this->getVersion(torPath);
    qDebug() << QString("Tor versions: embedded %1, filesystem %2").arg(embeddedVersion.toString(), filesystemVersion.toString());
    if (SemanticVersion::isValid(filesystemVersion) && (embeddedVersion > filesystemVersion)) {
        qInfo() << "Embedded version is newer, overwriting.";
        QFile::setPermissions(torPath, QFile::ReadOther | QFile::WriteOther);
        if (!QFile::remove(torPath)) {
            qWarning() << "Unable to remove old Tor binary";
            return false;
        }
    }

    if (embeddedVersion > filesystemVersion) {
        QDirIterator it(":/assets/tor", QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString assetFile = it.next();
            QFileInfo assetFileInfo = QFileInfo(assetFile);
            QFile f(assetFile);
            QString filePath = QDir(this->torDir).filePath(assetFileInfo.fileName());
            f.copy(filePath);
            f.close();
        }
        qInfo() << "Wrote Tor binaries to: " << this->torDir;
    }

#if defined(Q_OS_UNIX)
    QFile tor(this->torPath);
    tor.setPermissions(QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther
    | QFile::ReadOwner | QFile::ReadGroup | QFile::ReadOther);
#endif

    m_unpacked = true;
    return true;
}

bool TorManager::isLocalTor() {
    return m_localTor;
}

bool TorManager::isStarted() {
    return m_started;
}

bool TorManager::isAlreadyRunning() {
    return m_alreadyRunning;
}

// Determines whether Feather should start its own managed Tor daemon.
// Returns false (use external Tor) when any of these conditions are met:
//   - Running under torsocks (detected via LD_PRELOAD / DYLD_INSERT_LIBRARIES)
//   - Running on Tails or Whonix (these OSes manage their own Tor)
//   - Built without embedded Tor binary (no HAS_TOR_BIN or TOR_INSTALLED)
//   - Proxy is not set to Tor
//   - --use-local-tor flag or useLocalTor config is set
//   - A Tor daemon is already listening on socks5Port (default 9050)
//   - A service is already listening on featherTorPort (default 19450), sets m_alreadyRunning
// Sets m_alreadyRunning = true if port 19450 is occupied, causing checkConnection()
// to probe that port instead of socks5Port.
bool TorManager::shouldStartTorDaemon() {
    QString torHost = conf()->get(Config::socks5Host).toString();
    quint16 torPort = conf()->get(Config::socks5Port).toString().toUShort();
    QString torHostPort = QString("%1:%2").arg(torHost, QString::number(torPort));
    m_alreadyRunning = false;

    // Don't start a Tor daemon if Feather is run with Torsocks
    if (Utils::isTorsocks()) {
        return false;
    }

    // Don't start a Tor daemon on privacy OSes (they manage their own Tor instance)
    if (TailsOS::detect() || WhonixOS::detect()) {
        return false;
    }

    // Don't start a Tor daemon if we don't have one
#if !defined(HAS_TOR_BIN) && !defined(TOR_INSTALLED)
    qWarning() << "Feather built without embedded Tor. Assuming --use-local-tor";
    return false;
#endif

    // Don't start a Tor daemon if our proxy config isn't set to Tor
    if (conf()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        return false;
    }

    // Don't start a Tor daemon if --use-local-tor is specified
    if (conf()->get(Config::useLocalTor).toBool()) {
        return false;
    }

    if (m_started) {
        return true;
    }

    // Don't start a Tor daemon if one is already running
    if (Utils::portOpen(torHost, torPort)) {
        return false;
    }

    bool unpacked = this->unpackBins();
    if (!unpacked) {
        // Don't try to start a Tor daemon if unpacking failed
        qWarning() << "Error unpacking embedded Tor. Assuming --use-local-tor";
        this->setErrorMessage("Error unpacking embedded Tor. Assuming --use-local-tor");
        return false;
    }

    // Tor daemon (or other service) is already running on our port (19450)

    if (Utils::portOpen(featherTorHost, featherTorPort)) {
        m_alreadyRunning = true;
        return false;
    }

    return true;
}

SemanticVersion TorManager::getVersion(const QString &fileName) {
    QProcess process;
    process.setProcessChannelMode(QProcess::MergedChannels);
    process.start(this->torPath, QStringList() << "--version");
    process.waitForFinished(-1);
    QString output = process.readAllStandardOutput();

    if(output.isEmpty()) {
        qWarning() << "Could not grab Tor version";
        return SemanticVersion();
    }

    return SemanticVersion::fromString(output);
}

void TorManager::setErrorMessage(const QString &msg) {
    this->errorMsg = msg;
    emit statusChanged(msg);
}

TorManager* TorManager::instance()
{
    if (!m_instance) {
        m_instance = new TorManager(QCoreApplication::instance());
    }

    return m_instance;
}

TorManager::~TorManager() = default;
