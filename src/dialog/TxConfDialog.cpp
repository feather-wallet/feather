// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "TxConfDialog.h"
#include "ui_TxConfDialog.h"

#include <QMessageBox>

#include "model/ModelUtils.h"
#include "TxConfAdvDialog.h"
#include "constants.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"

TxConfDialog::TxConfDialog(QSharedPointer<AppContext> ctx, PendingTransaction *tx, const QString &address, const QString &description, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TxConfDialog)
        , m_ctx(std::move(ctx))
        , m_tx(tx)
        , m_address(address)
        , m_description(description)
{
    ui->setupUi(this);

    ui->label_warning->setText("You are about to send a transaction.\nVerify the information below.");
    ui->label_note->hide();

    QString preferredCur = config()->get(Config::preferredFiatCurrency).toString();

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

    ui->label_amount->setFont(ModelUtils::getMonospaceFont());
    ui->label_fee->setFont(ModelUtils::getMonospaceFont());
    ui->label_total->setFont(ModelUtils::getMonospaceFont());

    ui->label_amount->setText(QString("%1 (%2 %3)").arg(amounts[0], amounts_fiat[0], preferredCur));
    ui->label_fee->setText(QString("%1 (%2 %3)").arg(amounts[1], amounts_fiat[1], preferredCur));
    ui->label_total->setText(QString("%1 (%2 %3)").arg(amounts[2], amounts_fiat[2], preferredCur));

    auto subaddressIndex = m_ctx->wallet->subaddressIndex(address);
    QString addressExtra;

    ui->label_address->setText(ModelUtils::displayAddress(address, 2));
    ui->label_address->setFont(ModelUtils::getMonospaceFont());
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


    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Send");

    connect(ui->btn_Advanced, &QPushButton::clicked, this, &TxConfDialog::setShowAdvanced);

    m_ctx->txCache[tx->txid()[0]] = tx->signedTxToHex(0);
    this->adjustSize();
}

void TxConfDialog::setShowAdvanced() {
    this->showAdvanced = true;
    QDialog::reject();
}

TxConfDialog::~TxConfDialog() {
    delete ui;
}
