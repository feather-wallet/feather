// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_CLI_H
#define FEATHER_CLI_H

#include <QtCore>
#include "appcontext.h"

class CLI : public QObject
{
    Q_OBJECT

public:
    enum Mode {
        ExportContacts,
        ExportTxHistory,
        BruteforcePassword
    };

    explicit CLI(Mode mode, QCommandLineParser *cmdargs, QObject *parent = nullptr);

private slots:
    void onWalletOpened(Wallet *wallet);

private:
    void finished(const QString &message);

    Mode m_mode;
    QCommandLineParser *m_cmdargs;
    WalletManager *m_walletManager;
};

#endif //FEATHER_CLI_H
