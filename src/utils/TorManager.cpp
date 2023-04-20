// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "utils/TorManager.h"

#include <QScreen>
#include <QDesktopServices>
#include <QRegularExpression>

#include "utils/config.h"
#include "utils/Utils.h"
#include "utils/os/tails.h"
#include "utils/os/whonix.h"
#include "config-feather.h"

TorManager::TorManager(QObject *parent)
    : QObject(parent)
    , m_checkConnectionTimer(new QTimer(this))
    , m_process(new QProcess(this))
{
    connect(m_checkConnectionTimer, &QTimer::timeout, this, &TorManager::checkConnection);

    this->torDir = Config::defaultConfigDir().filePath("tor");
#if defined(PLATFORM_INSTALLER)
    // When installed, use directory relative to application path.
    this->torDir = QDir(Utils::applicationPath()).filePath("tor");
#endif

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

    featherTorPort = config()->get(Config::torManagedPort).toString().toUShort();
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

    qDebug() << QString("Start process: %1").arg(this->torPath);

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

void TorManager::checkConnection() {
    qDebug() << "Checking Tor connection";
    // We might not be able to connect to localhost if torsocks is used to start feather
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

    else if (config()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        this->setConnectionState(false);
    }

    else if (m_localTor) {
        qDebug() << "Tor daemon is local";
        QString host = config()->get(Config::socks5Host).toString();
        quint16 port = config()->get(Config::socks5Port).toString().toUShort();
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

#if defined(PLATFORM_INSTALLER)
    // We don't need to unpack if Tor was installed using the installer
    return true;
#endif

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

bool TorManager::shouldStartTorDaemon() {
    QString torHost = config()->get(Config::socks5Host).toString();
    quint16 torPort = config()->get(Config::socks5Port).toString().toUShort();
    QString torHostPort = QString("%1:%2").arg(torHost, QString::number(torPort));

    // Don't start a Tor daemon if Feather is run with Torsocks
    if (Utils::isTorsocks()) {
        return false;
    }

    // Don't start a Tor daemon on privacy OSes
    if (TailsOS::detect() || WhonixOS::detect()) {
        return false;
    }

    // Don't start a Tor daemon if we don't have one
#if !defined(HAS_TOR_BIN) && !defined(PLATFORM_INSTALLER)
    qWarning() << "Feather built without embedded Tor. Assuming --use-local-tor";
    return false;
#endif

    // Don't start a Tor daemon if our proxy config isn't set to Tor
    if (config()->get(Config::proxy).toInt() != Config::Proxy::Tor) {
        return false;
    }

    // Don't start a Tor daemon if --use-local-tor is specified
    if (config()->get(Config::useLocalTor).toBool()) {
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
