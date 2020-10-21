// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_CLI_H
#define FEATHER_CLI_H

#include <QtCore>
#include "appcontext.h"

enum CLIMode {
    CLIModeExportContacts,
    CLIModeExportTxHistory
};

class CLI : public QObject
{
    Q_OBJECT
public:
    CLIMode mode;
    explicit CLI(AppContext *ctx, QObject *parent = nullptr);
    ~CLI() override;

public slots:
    void run();

    //libwalletqt
    void onWalletOpened();
    void onWalletOpenedError(const QString& err);
    void onWalletOpenPasswordRequired(bool invalidPassword, const QString &path);

private:
    AppContext *ctx;

private slots:
    void finished(const QString &msg);
    void finishedError(const QString &err);

signals:
    void closeApplication();
};

#endif //FEATHER_CLI_H
