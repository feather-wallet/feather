// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "signverifydialog.h"
#include "ui_signverifydialog.h"
#include "utils/utils.h"

#include <QMessageBox>

SignVerifyDialog::SignVerifyDialog(Wallet *wallet, QWidget *parent)
        : QDialog(parent)
        , m_wallet(wallet)
        , ui(new Ui::SignVerifyDialog)
{
    ui->setupUi(this);

    connect(ui->btn_Sign, &QPushButton::clicked, this, &SignVerifyDialog::signMessage);
    connect(ui->btn_Verify, &QPushButton::clicked, this, &SignVerifyDialog::verifyMessage);
    connect(ui->btn_Copy, &QPushButton::clicked, this, &SignVerifyDialog::copyToClipboard);

    connect(ui->message, &QPlainTextEdit::textChanged, [this](){ui->btn_Copy->setVisible(false);});
    connect(ui->address, &QLineEdit::textEdited, [this](){ui->btn_Copy->setVisible(false);});
    connect(ui->signature, &QLineEdit::textEdited, [this](){ui->btn_Copy->setVisible(false);});

    ui->address->setText(m_wallet->address(0, 0));
    ui->address->setCursorPosition(0);

    ui->btn_Copy->setVisible(false);
}

void SignVerifyDialog::signMessage() {
    QString signature = m_wallet->signMessage(ui->message->toPlainText(), false, ui->address->text());

    if (signature.isEmpty()) {
        QMessageBox::information(this, "Information", m_wallet->errorString());
        return;
    }

    ui->signature->setText(signature);
    ui->btn_Copy->setVisible(true);
}

void SignVerifyDialog::verifyMessage() {
    bool verified = m_wallet->verifySignedMessage(ui->message->toPlainText(), ui->address->text(), ui->signature->text());
    verified ? QMessageBox::information(this, "Information", "Signature is valid")
             : QMessageBox::warning(this, "Warning", "Signature failed to verify");
}

void SignVerifyDialog::copyToClipboard() {
    QStringList sig;
    sig << ui->message->toPlainText() << ui->address->text() << ui->signature->text();
    Utils::copyToClipboard(sig.join("\n"));
}

SignVerifyDialog::~SignVerifyDialog()
{
    delete ui;
}