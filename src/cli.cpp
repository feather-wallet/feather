// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "cli.h"

// libwalletqt
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/WalletManager.h"
#include "model/AddressBookModel.h"
#include "model/TransactionHistoryModel.h"
#include "utils/brute.h"

CLI::CLI(AppContext *ctx, QObject *parent) :
        QObject(parent),
        ctx(ctx) {
    connect(this->ctx, &AppContext::walletOpened, this, &CLI::onWalletOpened);
    connect(this->ctx, &AppContext::walletOpenedError, this, &CLI::onWalletOpenedError);
    connect(this->ctx, &AppContext::walletOpenPasswordNeeded, this, &CLI::onWalletOpenPasswordRequired);
}

void CLI::run() {
    if (mode == CLIMode::ExportContacts || mode == CLIMode::ExportTxHistory)
    {
        if(!ctx->cmdargs->isSet("wallet-file"))
            return this->finishedError("--wallet-file argument missing");
        if(!ctx->cmdargs->isSet("password"))
            return this->finishedError("--password argument missing");
        ctx->onOpenWallet(ctx->cmdargs->value("wallet-file"), ctx->cmdargs->value("password"));
    }
    else if (mode == CLIMode::BruteforcePassword)
    {
        QString keys_file = ctx->cmdargs->value("bruteforce-password");
        if (!keys_file.endsWith(".keys")) {
            return this->finishedError("Wallet file does not end with .keys");
        }

        QStringList words;
        if (ctx->cmdargs->isSet("bruteforce-dict")) {
            QString data = Utils::barrayToString(Utils::fileOpen(ctx->cmdargs->value("bruteforce-dict")));
            words = data.split("\n");
        }

        if (!ctx->cmdargs->isSet("bruteforce-chars")) {
            return this->finishedError("--bruteforce-chars argument missing");
        }
        QString chars = ctx->cmdargs->value("bruteforce-chars");

        brute b(chars.toStdString());
        if (words.isEmpty()) {
            qDebug() << "No dictionairy specified, bruteforcing all chars";
            while (true) {
                QString pass = QString::fromStdString(b.next());
                if (ctx->walletManager->verifyWalletPassword(keys_file, pass, false)) {
                    this->finished(QString("Found password: %1").arg(pass));
                    break;
                }
                qDebug() << pass;
            }
        }
        else {
            bruteword bb(chars.toStdString());
            bool foundPass = false;
            for (auto word: words) {
                if (word.isEmpty()) {
                    continue;
                }
                bb.setWord(word.toStdString());

                while (true) {
                    QString pass = QString::fromStdString(bb.next());
                    if (pass == "") {
                        break;
                    }
                    if (ctx->walletManager->verifyWalletPassword(keys_file, pass, false)) {
                        this->finished(QString("Found password: %1").arg(pass));
                        foundPass = true;
                        break;
                    }
                    qDebug() << pass;
                }
                if (foundPass) {
                    break;
                }
            }

            if (!foundPass) {
                this->finished("Search space exhausted");
            }
        }
    }
}

void CLI::onWalletOpened() {
    if(mode == CLIMode::ExportContacts){
        auto *model = ctx->currentWallet->addressBookModel();
        auto fn = ctx->cmdargs->value("export-contacts");
        if(model->writeCSV(fn))
            this->finished(QString("Address book exported to %1").arg(fn));
        else
            this->finishedError("Address book export failure");
    } else if(mode == ExportTxHistory) {
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
    if(mode == CLIMode::ExportContacts ||
       mode == CLIMode::ExportTxHistory)
        return this->finishedError(err);
}

void CLI::onWalletOpenPasswordRequired(bool invalidPassword, const QString &path) {
    if(mode == CLIMode::ExportContacts ||
       mode == CLIMode::ExportTxHistory)
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
