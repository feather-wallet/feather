// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>

#include "viewonlydialog.h"
#include "ui_viewonlydialog.h"

ViewOnlyDialog::ViewOnlyDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ViewOnlyDialog)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    ui->label_restoreHeight->setText(QString::number(m_ctx->wallet->getWalletCreationHeight()));
    ui->label_primaryAddress->setText(m_ctx->wallet->address(0, 0));
    ui->label_secretViewKey->setText(m_ctx->wallet->getSecretViewKey());

    connect(ui->btn_Copy, &QPushButton::clicked, this, &ViewOnlyDialog::copyToClipboad);
    connect(ui->btn_Save, &QPushButton::clicked, this, &ViewOnlyDialog::onWriteViewOnlyWallet);

    ui->btn_Save->setEnabled(!m_ctx->wallet->viewOnly());
    this->adjustSize();
}

void ViewOnlyDialog::onWriteViewOnlyWallet(){
    QString fn = QFileDialog::getSaveFileName(this, "Save .keys wallet file", Utils::defaultWalletDir(), "Monero wallet (*.keys)");
    if(fn.isEmpty()) return;
    if(!fn.endsWith(".keys")) fn += ".keys";

    QString passwd;
    QInputDialog passwordDialog(this);
    passwordDialog.setInputMode(QInputDialog::TextInput);
    passwordDialog.setTextEchoMode(QLineEdit::Password);
    passwordDialog.setWindowTitle("View-Only wallet password");
    passwordDialog.setLabelText("Protect this view-only wallet with a password?");
    passwordDialog.resize(300, 100);
    if((bool)passwordDialog.exec())
        passwd = passwordDialog.textValue();

    m_ctx->wallet->createViewOnly(fn, passwd);

    QMessageBox::information(this, "Information", "View-only wallet successfully written to disk.");
}

void ViewOnlyDialog::copyToClipboad() {
    QString text = "";
    text += QString("Address: %1\n").arg(ui->label_primaryAddress->text());
    text += QString("Secret view key: %1\n").arg(ui->label_secretViewKey->text());
    text += QString("Restore height: %1\n").arg(ui->label_restoreHeight->text());
    Utils::copyToClipboard(text);
}

ViewOnlyDialog::~ViewOnlyDialog()
{
    delete ui;
}
