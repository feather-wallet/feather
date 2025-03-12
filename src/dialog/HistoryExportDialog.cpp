// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "HistoryExportDialog.h"
#include "ui_HistoryExportDialog.h"

#include <QFileDialog>

#include "Utils.h"
#include "WalletManager.h"
#include "libwalletqt/Wallet.h"
#include "TransactionHistory.h"
#include "utils/AppData.h"

HistoryExportDialog::HistoryExportDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::HistoryExportDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_export, &QPushButton::clicked, this, &HistoryExportDialog::exportHistory);

    connect(ui->radio_everything, &QRadioButton::toggled, [this](bool toggled) {
        if (!toggled) return;
        this->setEverything();
    });

    connect(ui->radio_YTD, &QRadioButton::toggled, [this](bool toggled) {
        if (!toggled) return;
        ui->date_min->setDate(QDate(QDate::currentDate().year(), 1, 1));
        ui->date_max->setDate(QDate::currentDate());
    });

    connect(ui->radio_lastDays, &QRadioButton::toggled, [this](bool toggled) {
        ui->spin_days->setEnabled(toggled);
        if (!toggled) return;
        ui->date_min->setDate(QDate::currentDate().addDays(-ui->spin_days->value() + 1));
        ui->date_max->setDate(QDate::currentDate());
    });

    connect(ui->spin_days, &QSpinBox::valueChanged, [this] {
        ui->date_min->setDate(QDate::currentDate().addDays(-ui->spin_days->value() + 1));
        ui->date_max->setDate(QDate::currentDate());
    });

    connect(ui->radio_lastMonth, &QRadioButton::toggled, [this](bool toggled) {
        if (!toggled) return;
        ui->date_min->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1).addMonths(-1));
        ui->date_max->setDate(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1).addDays(-1));
    });

    connect(ui->radio_lastYear, &QRadioButton::toggled, [this](bool toggled) {
        if (!toggled) return;
        ui->date_min->setDate(QDate(QDate::currentDate().year() - 1, 1, 1));
        ui->date_max->setDate(QDate(QDate::currentDate().year(), 1, 1).addDays(-1));
    });

    connect(ui->radio_customRange, &QRadioButton::toggled, [this](bool toggled) {
        ui->date_min->setEnabled(toggled);
        ui->date_max->setEnabled(toggled);
    });

    this->setEverything();

    this->adjustSize();
}

void HistoryExportDialog::setEverything()
{
    ui->date_min->setDate(QDate(2014,4,18));
    ui->date_max->setDate(QDate::currentDate());
}

void HistoryExportDialog::exportHistory()
{
    QString wallet_name = m_wallet->walletName();
    QString csv_file_name = QString("/history_export_%1.csv").arg(wallet_name);

    QString filePath = QFileDialog::getSaveFileName(this, "Save CSV file", QDir::homePath() + csv_file_name, "CSV (*.csv)");
    if (filePath.isEmpty())
        return;
    if (!filePath.endsWith(".csv"))
        filePath += ".csv";
    QFileInfo fileInfo(filePath);
    QDir dir = fileInfo.absoluteDir();

    if (!dir.exists()) {
        Utils::showError(this, "Unable to export transaction history", QString("Path does not exist: %1").arg(dir.absolutePath()));
        return;
    }

    auto num_transactions = m_wallet->history()->count();

    QList<QPair<uint64_t, QString>> csvData;

    for (int i = 0; i < num_transactions; i++) {
        const TransactionRow& tx = m_wallet->history()->transaction(i);

        QDate minimumDate = ui->date_min->date();
        if (tx.timestamp.date() < minimumDate) {
            continue;
        }

        QDate maximumDate = ui->date_max->date();
        if (tx.timestamp.date() > maximumDate) {
            continue;
        }

        if (ui->check_excludePending->isChecked() && tx.pending) {
            continue;
        }

        if (ui->check_excludeFailed->isChecked() && tx.failed) {
            continue;
        }

        if (ui->radio_incomingTransactions->isChecked() && tx.direction != TransactionRow::Direction_In) {
            continue;
        }

        if (ui->radio_outgoingTransactions->isChecked() && tx.direction != TransactionRow::Direction_Out) {
            continue;
        }

        if (ui->radio_coinbaseTransactions->isChecked() && !tx.coinbase) {
            continue;
        }

        QString date = QString("%1T%2Z").arg(tx.date(), tx.time());

        QString direction = QString("");
        if (tx.direction == TransactionRow::Direction_In)
            direction = QString("in");
        else if (tx.direction == TransactionRow::Direction_Out)
            direction = QString("out");
        else
            continue;  // skip TransactionInfo::Direction_Both

        QString balanceDelta = WalletManager::displayAmount(abs(tx.balanceDelta));
        if (tx.direction == TransactionRow::Direction_Out) {
            balanceDelta = "-" + balanceDelta;
        }

        QString paymentId = tx.paymentId;
        if (paymentId == "0000000000000000") {
            paymentId = "";
        }

        const double usd_price = appData()->txFiatHistory->get(tx.timestamp.toString("yyyyMMdd"));
        double fiat_price = usd_price * tx.amountDouble();
        QString fiatAmount = (usd_price > 0) ? QString::number(fiat_price, 'f', 2) : "?";

        QString line = QString(R"(%1,%2,"%3",%4,"%5",%6,%7,%8,"%9","%10","%11","%12","%13")")
        .arg(QString::number(tx.blockHeight),
             QString::number(tx.timestamp.toSecsSinceEpoch()),
             date,
             QString::number(tx.subaddrAccount),
             direction,
             balanceDelta,
             tx.displayAmount(),
             tx.displayFee(),
             tx.hash,
             tx.description,
             paymentId,
             fiatAmount,
             "USD");
        csvData.append({tx.blockHeight, line});
    }

    std::sort(csvData.begin(), csvData.end(), [](const QPair<uint64_t, QString> &tx1, const QPair<uint64_t, QString> &tx2){
        return tx1.first < tx2.first;
    });

    QString csvString = "blockHeight,timestamp,date,accountIndex,direction,balanceDelta,amount,fee,txid,description,paymentId,fiatAmount,fiatCurrency";
    for (const auto& data : csvData) {
        csvString += "\n" + data.second;
    }

    bool success = Utils::fileWrite(filePath, csvString);

    if (!success) {
        Utils::showError(this, "Unable to export transaction history", QString("No permission to write to: %1").arg(filePath));
        return;
    }

    Utils::showInfo(this, "CSV export", QString("Transaction history exported to:\n\n%1").arg(filePath));
}

HistoryExportDialog::~HistoryExportDialog() = default;
