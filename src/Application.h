// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_APPLICATION_H
#define FEATHER_APPLICATION_H

#include <QApplication>
#include <QtNetwork/qlocalserver.h>

class QLockFile;

class Application : public QApplication {
    Q_OBJECT

public:
    Application(int& argc, char** argv);
    ~Application() override;

    bool isAlreadyRunning() const;

signals:
    void anotherInstanceStarted();

private slots:
    void processIncomingConnection();

private:
    bool m_alreadyRunning;
    QLockFile* m_lockFile;
    QLocalServer m_lockServer;
    QString m_socketName;
};


#endif //FEATHER_APPLICATION_H
