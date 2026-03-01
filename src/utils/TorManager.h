// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_TOR_H
#define FEATHER_TOR_H

#include <QObject>
#include <QProcess>
#include <QTimer>

#include "utils/SemanticVersion.h"

class TorManager : public QObject
{
Q_OBJECT

public:
    explicit TorManager(QObject *parent = nullptr);
    ~TorManager() override;

    void init();
    void start();
    void stop();
    bool unpackBins();
    bool isLocalTor();
    bool isStarted();
    bool isAlreadyRunning();
    SemanticVersion getVersion(const QString &fileName);

    static TorManager* instance();

    bool torConnected = false;  // True after Tor bootstraps (100%) or is assumed connected
                                // (torsocks/Whonix/Tails). Checked every 5s by checkConnection().

    // Host and port for Feather's MANAGED Tor daemon. This is separate from the user-configured
    // socks5Host:socks5Port (default 9050) to avoid port conflicts with a system Tor daemon.
    // When connecting to a node, the proxy address is chosen in Nodes::connectToNode():
    //   - Managed Tor (or m_alreadyRunning): uses featherTorHost:featherTorPort
    //   - Local/system Tor: uses socks5Host:socks5Port from config
    QString featherTorHost = "127.0.0.1";
    quint16 featherTorPort = 19450;

    QString torDir;
    QString torPath;
    QString torDataPath;

    QString torLogs;
    QString errorMsg = "";

signals:
    void connectionStateChanged(bool connected);
    void statusChanged(QString reason);
    void logsUpdated();

private slots:
    void stateChanged(QProcess::ProcessState);
    void handleProcessOutput();
    void handleProcessError(QProcess::ProcessError error);
    void checkConnection();

private:
    bool shouldStartTorDaemon();
    void setConnectionState(bool connected);
    void setErrorMessage(const QString &msg);

    static QPointer<TorManager> m_instance;

    QProcess *m_process;
    int m_restarts = 0;          // Number of start() attempts. If > 4, Tor gives up permanently
                                 // with "maximum retries exceeded" and no further user notification.
    bool m_stopRetries = false;  // Set to true on FailedToStart error. Prevents automatic restart
                                 // via stateChanged() when the binary itself is missing/broken.
    bool m_localTor = false;     // True when using system Tor (not managed). Set by init() based
                                 // on shouldStartTorDaemon(). Affects which port checkConnection() probes.
    bool m_started = false;      // True after start() launches the managed Tor process.
    bool m_unpacked = false;     // True after embedded Tor binary has been extracted to disk.
    bool m_alreadyRunning = false; // True if featherTorPort (19450) was already occupied when
                                   // shouldStartTorDaemon() ran. Causes checkConnection() to probe
                                   // that port instead of socks5Port.
    QTimer *m_checkConnectionTimer; // Fires every 5 seconds to call checkConnection().
};

inline TorManager* torManager()
{
    return TorManager::instance();
}

#endif //FEATHER_TOR_H