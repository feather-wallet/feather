// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "cli.h"

// libwalletqt
#include "Wallet.h"
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/SubaddressAccount.h"
#include "libwalletqt/Subaddress.h"
#include "libwalletqt/AddressBook.h"
#include "libwalletqt/Coins.h"
#include "model/AddressBookModel.h"
#include "model/TransactionHistoryModel.h"
#include "model/SubaddressAccountModel.h"
#include "model/SubaddressModel.h"
#include "model/CoinsModel.h"

CLI::CLI(AppContext *ctx, QObject *parent) :
        QObject(parent),
        ctx(ctx) {
    connect(this->ctx, &AppContext::walletOpened, this, &CLI::onWalletOpened);
    connect(this->ctx, &AppContext::walletOpenedError, this, &CLI::onWalletOpenedError);
    connect(this->ctx, &AppContext::walletOpenPasswordNeeded, this, &CLI::onWalletOpenPasswordRequired);
}

void CLI::run() {
    if(mode == CLIMode::CLIModeExportContacts ||
            mode == CLIMode::CLIModeExportTxHistory) {
        if(!ctx->cmdargs->isSet("wallet-file")) return this->finishedError("--wallet-file argument missing");
        if(!ctx->cmdargs->isSet("password")) return this->finishedError("--password argument missing");
        ctx->onOpenWallet(ctx->cmdargs->value("wallet-file"), ctx->cmdargs->value("password"));
    }
}

void CLI::onWalletOpened() {
    if(mode == CLIMode::CLIModeExportContacts){
        auto *model = ctx->currentWallet->addressBookModel();
        auto fn = ctx->cmdargs->value("export-contacts");
        if(model->writeCSV(fn))
            this->finished(QString("Address book exported to %1").arg(fn));
        else
            this->finishedError("Address book export failure");
    } else if(mode == CLIModeExportTxHistory) {
        ctx->currentWallet->history()->refresh(ctx->currentWallet->currentSubaddressAccount());
        auto *model = ctx->currentWallet->history();
        auto fn = ctx->cmdargs->value("export-txhistory");
        if(model->writeCSV(fn))
            this->finished(QString("Transaction history exported to %1").arg(fn));
        else
            this->finishedError("Transaction history export failure");
    }
}

void CLI::onWalletOpenedError(const QString &err) {
    if(mode == CLIMode::CLIModeExportContacts ||
       mode == CLIMode::CLIModeExportTxHistory)
        return this->finishedError(err);
}

void CLI::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    if(mode == CLIMode::CLIModeExportContacts ||
       mode == CLIMode::CLIModeExportTxHistory)
        return this->finishedError("invalid password");
}

void CLI::finished(const QString &msg){
    qInfo() << msg;
    emit closeApplication();
}

void CLI::finishedError(const QString &err) {
    qCritical() << err;
    emit closeApplication();
}

CLI::~CLI() {
    ctx->disconnect();
    delete ctx;
}
