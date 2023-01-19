// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WalletUnlockWidget.h"
#include "ui_WalletUnlockWidget.h"

#include <QKeyEvent>
#include <QPushButton>

WalletUnlockWidget::WalletUnlockWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::WalletUnlockWidget)
{
    ui->setupUi(this);
    this->reset();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setAutoDefault(true);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &WalletUnlockWidget::tryUnlock);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &WalletUnlockWidget::closeWallet);
}

void WalletUnlockWidget::setWalletName(const QString &walletName) {
    ui->label_fileName->setText(walletName);
}

void WalletUnlockWidget::reset() {
    ui->label_incorrectPassword->hide();
    ui->line_password->setText("");
    ui->line_password->setFocus();
}

void WalletUnlockWidget::incorrectPassword() {
    ui->label_incorrectPassword->show();
    ui->line_password->clear();
}

void WalletUnlockWidget::tryUnlock() {
    emit unlockWallet(ui->line_password->text());
}

void WalletUnlockWidget::keyPressEvent(QKeyEvent *e) {
    switch (e->key()) {
        case Qt::Key_Enter:
        case Qt::Key_Return:
            ui->buttonBox->accepted();
            e->ignore();
            break;
        case Qt::Key_Escape:
            ui->buttonBox->rejected();
            e->ignore();
            break;
        default:
            e->ignore();
    }
}

WalletUnlockWidget::~WalletUnlockWidget() = default;