// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "Application.h"

#include "config.h"

#include <QLocalSocket>
#include <QLockFile>
#include <QStandardPaths>

namespace
{
    constexpr int WaitTimeoutMSec = 150;
    const char BlockSizeProperty[] = "blockSize";
} // namespace

Application::Application(int& argc, char** argv)
        : QApplication(argc, argv)
        , m_alreadyRunning(false)
        , m_lockFile(nullptr)
{
    QString userName = qgetenv("USER");
    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");
    }
    QString identifier = "feather";
    if (!userName.isEmpty()) {
        identifier += "-" + userName;
    }

    QString lockName = identifier + ".lock";
    m_socketName = identifier + ".socket";

    // According to documentation we should use RuntimeLocation on *nixes, but even Qt doesn't respect
    // this and creates sockets in TempLocation, so let's be consistent.
    m_lockFile = new QLockFile(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/" + lockName);
    m_lockFile->setStaleLockTime(0);
    m_lockFile->tryLock();

    m_lockServer.setSocketOptions(QLocalServer::UserAccessOption);
    connect(&m_lockServer, SIGNAL(newConnection()), this, SIGNAL(anotherInstanceStarted()));
    connect(&m_lockServer, &QLocalServer::newConnection, this, &Application::processIncomingConnection);

    switch (m_lockFile->error()) {
        case QLockFile::NoError:
            // No existing lock was found, start listener
            m_lockServer.listen(m_socketName);
            break;
        case QLockFile::LockFailedError: {
            // Attempt to connect to the existing instance
            QLocalSocket client;
            for (int i = 0; i < 3; ++i) {
                client.connectToServer(m_socketName);
                if (client.waitForConnected(WaitTimeoutMSec)) {
                    // Connection succeeded, this will raise the existing window if minimized
                    client.abort();
                    m_alreadyRunning = true;
                    break;
                }
            }

            if (!m_alreadyRunning) {
                // If we get here then the original instance is likely dead
                qWarning() << "Existing single-instance lock file is invalid. Launching new instance.";

                // forceably reset the lock file
                m_lockFile->removeStaleLockFile();
                m_lockFile->tryLock();
                // start the listen server
                m_lockServer.listen(m_socketName);
            }
            break;
        }
        default:
            qWarning() << "The lock file could not be created. Single-instance mode disabled.";
    }
}

Application::~Application()
{
    qDebug() << "~Application";
    if (m_lockFile) {
        m_lockFile->unlock();
        delete m_lockFile;
    }
}

bool Application::isAlreadyRunning() const
{
    return m_alreadyRunning;
}

void Application::processIncomingConnection()
{
    qDebug() << "We got an incoming connection";
    if (m_lockServer.hasPendingConnections()) {
        QLocalSocket* socket = m_lockServer.nextPendingConnection();
        socket->setProperty(BlockSizeProperty, 0);
    }
}
