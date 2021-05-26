// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "cli.h"

// libwalletqt
#include "libwalletqt/TransactionHistory.h"
#include "libwalletqt/WalletManager.h"
#include "model/AddressBookModel.h"
#include "model/TransactionHistoryModel.h"
#include "utils/brute.h"
#include "constants.h"

CLI::CLI(Mode mode, QCommandLineParser *cmdargs, QObject *parent)
    : QObject(parent)
    , m_mode(mode)
    , m_cmdargs(cmdargs)
{
    m_walletManager = WalletManager::instance();
    connect(m_walletManager, &WalletManager::walletOpened, this, &CLI::onWalletOpened);

    if (m_mode == Mode::ExportContacts || m_mode == Mode::ExportTxHistory)
    {
        if (!cmdargs->isSet("wallet-file")) {
            this->finished("--wallet-file argument missing");
            return;
        }
        if (!cmdargs->isSet("password")) {
            this->finished("--password argument missing");
            return;
        }

        QString walletFile = cmdargs->value("wallet-file");
        QString password = cmdargs->value("password");


        m_walletManager->openWalletAsync(walletFile, password, constants::networkType);
    }
    else if (mode == Mode::BruteforcePassword)
    {
        QString keys_file = m_cmdargs->value("bruteforce-password");
        if (!keys_file.endsWith(".keys")) {
            this->finished("Wallet file does not end with .keys");
            return;
        }

        QStringList words;
        if (m_cmdargs->isSet("bruteforce-dict")) {
            QString data = Utils::barrayToString(Utils::fileOpen(m_cmdargs->value("bruteforce-dict")));
            words = data.split("\n");
        }

        if (!m_cmdargs->isSet("bruteforce-chars")) {
            this->finished("--bruteforce-chars argument missing");
            return;
        }
        QString chars = m_cmdargs->value("bruteforce-chars");

        brute b(chars.toStdString());
        if (words.isEmpty()) {
            qDebug() << "No dictionairy specified, bruteforcing all chars";
            while (true) {
                QString pass = QString::fromStdString(b.next());
                if (m_walletManager->verifyWalletPassword(keys_file, pass, false)) {
                    this->finished(QString("Found password: %1").arg(pass));
                    break;
                }
                qDebug() << pass;
            }
        }
        else {
            bruteword bb(chars.toStdString());
            bool foundPass = false;
            for (const auto& word: words) {
                if (word.isEmpty()) {
                    continue;
                }
                bb.setWord(word.toStdString());

                while (true) {
                    QString pass = QString::fromStdString(bb.next());
                    if (pass == "") {
                        break;
                    }
                    if (m_walletManager->verifyWalletPassword(keys_file, pass, false)) {
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
    else {
        this->finished("Invalid mode");
    }
}

void CLI::onWalletOpened(Wallet *w) {
    QScopedPointer<Wallet> wallet{w};

    if (wallet->status() != Wallet::Status_Ok) {
        this->finished(wallet->errorString());
        return;
    }

    if (m_mode == Mode::ExportContacts) {
        auto *model = wallet->addressBookModel();
        QString fileName = m_cmdargs->value("export-contacts");
        if (model->writeCSV(fileName))
            this->finished(QString("Contacts exported to %1").arg(fileName));
        else
            this->finished("Failed to export contacts");
    }
    else if (m_mode == Mode::ExportTxHistory) {
        wallet->history()->refresh(wallet->currentSubaddressAccount());
        auto *model = wallet->history();
        QString fileName = m_cmdargs->value("export-txhistory");
        if (model->writeCSV(fileName))
            this->finished(QString("Transaction history exported to %1").arg(fileName));
        else
            this->finished("Failed to export transaction history");
    }
}

void CLI::finished(const QString &message) {
    qInfo() << message;
    QApplication::quit();
}
