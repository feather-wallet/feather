// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "txconfdialog.h"
#include "ui_txconfdialog.h"
#include "appcontext.h"
#include "utils/config.h"
#include "model/ModelUtils.h"

#include <QMessageBox>

TxConfDialog::TxConfDialog(PendingTransaction *tx, const QString &address, const QString &description, int mixin, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::TxConfDialog)
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

    connect(ui->btn_Advanced, &QPushButton::clicked, this, &TxConfDialog::showAdvanced);

    this->adjustSize();
}

void TxConfDialog::showAdvanced() {
    const auto amount = m_tx->amount() / AppContext::cdiv;
    const auto fee = m_tx->fee() / AppContext::cdiv;

    QString body = QString("Address: %2\n").arg(m_address.left(60));
    body += m_address.mid(60) + "\n";
    if(!m_description.isEmpty())
        body = QString("%1Description: %2\n").arg(body, m_description);
    body = QString("%1Amount: %2 XMR\n").arg(body, QString::number(amount));
    body = QString("%1Fee: %2 XMR\n").arg(body, QString::number(fee));
    body = QString("%1Ringsize: %2").arg(body, QString::number(m_mixin + 1));

    auto subaddrIndices = m_tx->subaddrIndices();
    for (int i = 0; i < subaddrIndices.count(); ++i){
        body = QString("%1\nSpending address index: %2").arg(body, QString::number(subaddrIndices.at(i).toInt()));
    }

    QMessageBox::information(this, "Transaction information", body);
}

TxConfDialog::~TxConfDialog() {
    delete ui;
}
