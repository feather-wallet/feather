// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "txconfdialog.h"
#include "ui_txconfdialog.h"
#include "appcontext.h"
#include "utils/config.h"
#include "model/ModelUtils.h"
#include "libwalletqt/WalletManager.h"
#include "txconfadvdialog.h"

#include <QMessageBox>

TxConfDialog::TxConfDialog(AppContext *ctx, PendingTransaction *tx, const QString &address, const QString &description, int mixin, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TxConfDialog)
        , m_ctx(ctx)
        , m_tx(tx)
        , m_address(address)
        , m_description(description)
        , m_mixin(mixin)
{
    ui->setupUi(this);

    ui->label_warning->setText("You are about to send a transaction.\nVerify the information below.");

    QString preferredCur = config()->get(Config::preferredFiatCurrency).toString();

    auto convert = [preferredCur](double amount){
        return QString::number(AppContext::prices->convert("XMR", preferredCur, amount), 'f', 2);
    };

    QString amount = WalletManager::displayAmount(tx->amount());
    QString amount_fiat = convert(tx->amount() / AppContext::cdiv);
    ui->label_amount->setText(QString("%1 (%2 %3)").arg(amount, amount_fiat, preferredCur));

    QString fee = WalletManager::displayAmount(tx->fee());
    QString fee_fiat = convert(tx->fee() / AppContext::cdiv);
    ui->label_fee->setText(QString("%1 (%2 %3)").arg(fee, fee_fiat, preferredCur));

    QString total = WalletManager::displayAmount(tx->amount() + tx->fee());
    QString total_fiat = convert((tx->amount() + tx->fee()) / AppContext::cdiv);
    ui->label_total->setText(QString("%1 (%2 %3)").arg(total, total_fiat, preferredCur));

    ui->label_address->setText(ModelUtils::displayAddress(address, 2));
    ui->label_address->setFont(ModelUtils::getMonospaceFont());
    ui->label_address->setToolTip(address);

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText("Send");

    connect(ui->btn_Advanced, &QPushButton::clicked, this, &TxConfDialog::setShowAdvanced);

    this->adjustSize();
}

void TxConfDialog::setShowAdvanced() {
    this->showAdvanced = true;
    QDialog::reject();
}

TxConfDialog::~TxConfDialog() {
    delete ui;
}
