// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TxDetailsSimple.h"
#include "ui_TxDetailsSimple.h"

#include "constants.h"
#include "libwalletqt/WalletManager.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/config.h"
#include "utils/Utils.h"

TxDetailsSimple::TxDetailsSimple(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::TxDetailsSimple)
{
    ui->setupUi(this);

    ui->label_amount->setFont(Utils::getMonospaceFont());
    ui->label_fee->setFont(Utils::getMonospaceFont());
    ui->label_total->setFont(Utils::getMonospaceFont());
}

void TxDetailsSimple::setDetails(Wallet *wallet, PendingTransaction *tx, const QString &address) {
    ui->label_note->hide();

    QString preferredCur = conf()->get(Config::preferredFiatCurrency).toString();

    auto convert = [preferredCur](double amount){
        return QString::number(appData()->prices.convert("XMR", preferredCur, amount), 'f', 2);
    };

    QString amount = WalletManager::displayAmount(tx->amount());
    QString fee = WalletManager::displayAmount(tx->fee());
    QString total = WalletManager::displayAmount(tx->amount() + tx->fee());
    QVector<QString> amounts = {amount, fee, total};
    int maxLength = Utils::maxLength(amounts);
    std::for_each(amounts.begin(), amounts.end(), [maxLength](QString& amount){amount = amount.rightJustified(maxLength, ' ');});

    QString amount_fiat = convert(tx->amount() / constants::cdiv);
    QString fee_fiat = convert(tx->fee() / constants::cdiv);
    QString total_fiat = convert((tx->amount() + tx->fee()) / constants::cdiv);
    QVector<QString> amounts_fiat = {amount_fiat, fee_fiat, total_fiat};
    int maxLengthFiat = Utils::maxLength(amounts_fiat);
    std::for_each(amounts_fiat.begin(), amounts_fiat.end(), [maxLengthFiat](QString& amount){amount = amount.rightJustified(maxLengthFiat, ' ');});

    ui->label_amount->setText(QString("%1 (%2 %3)").arg(amounts[0], amounts_fiat[0], preferredCur));
    ui->label_fee->setText(QString("%1 (%2 %3)").arg(amounts[1], amounts_fiat[1], preferredCur));
    ui->label_total->setText(QString("%1 (%2 %3)").arg(amounts[2], amounts_fiat[2], preferredCur));

    auto subaddressIndex = wallet->subaddressIndex(address);
    QString addressExtra;

    ui->label_address->setText(Utils::displayAddress(address, 2));
    ui->label_address->setFont(Utils::getMonospaceFont());
    ui->label_address->setToolTip(address);

    if (subaddressIndex.isValid()) {
        ui->label_note->setText("Note: this is a churn transaction.");
        ui->label_note->show();
        ui->label_address->setStyleSheet(ColorScheme::GREEN.asStylesheet(true));
        ui->label_address->setToolTip("Wallet receive address");
    }

    if (subaddressIndex.isPrimary()) {
        ui->label_address->setStyleSheet(ColorScheme::YELLOW.asStylesheet(true));
        ui->label_address->setToolTip("Wallet change/primary address");
    }

    if (tx->fee() > WalletManager::amountFromDouble(0.01)) {
        ui->label_fee->setStyleSheet(ColorScheme::RED.asStylesheet(true));
        ui->label_fee->setToolTip("Unrealistic fee. You may be connected to a malicious node.");
    }
}

TxDetailsSimple::~TxDetailsSimple() = default;