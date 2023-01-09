// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PasswordChangeDialog.h"
#include "ui_PasswordChangeDialog.h"

#include <QMessageBox>

PasswordChangeDialog::PasswordChangeDialog(QWidget *parent, Wallet *wallet)
        : WindowModalDialog(parent)
        , ui(new Ui::PasswordChangeDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    bool noPassword = wallet->getPassword().isEmpty();

    QString warning_str = noPassword ? "Your wallet is not password protected. Use this dialog to add a password to your wallet." :
                         "Your wallet is password protected and encrypted. Use this dialog to change your password.";
    ui->label_warning->setText(warning_str);

    QPixmap pixmap = noPassword ? QPixmap(":/assets/images/unlock.png") : QPixmap(":/assets/images/lock.png");
    ui->icon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    if (noPassword) {
        ui->label_currentPassword->hide();
        ui->lineEdit_currentPassword->hide();
    }

    connect(ui->lineEdit_newPassword, &QLineEdit::textChanged, this, &PasswordChangeDialog::passwordsMatch);
    connect(ui->lineEdit_confirmPassword, &QLineEdit::textChanged, this, &PasswordChangeDialog::passwordsMatch);

    connect(ui->btn_Cancel, &QPushButton::clicked, [this]{
       this->reject();
    });
    connect(ui->btn_OK, &QPushButton::clicked, this, &PasswordChangeDialog::setPassword);

    ui->label_match->setVisible(false);

    this->adjustSize();
}

void PasswordChangeDialog::passwordsMatch() {
    bool match = ui->lineEdit_newPassword->text() == ui->lineEdit_confirmPassword->text();
    ui->btn_OK->setEnabled(match);
    ui->label_match->setHidden(match);
}

void PasswordChangeDialog::setPassword() {
    QString currentPassword = ui->lineEdit_currentPassword->text();
    QString newPassword = ui->lineEdit_newPassword->text();

    if (currentPassword != m_wallet->getPassword()) {
        QMessageBox::warning(this, "Error", "Incorrect password");
        ui->lineEdit_currentPassword->setText("");
        ui->lineEdit_currentPassword->setFocus();
        return;
    }

    if (m_wallet->setPassword(newPassword)) {
        QMessageBox::information(this, "Information", "Password changed successfully");
        this->accept();
    }
    else {
        QMessageBox::warning(this, "Error", QString("Error: %1").arg(m_wallet->errorString()));
    }
}

PasswordChangeDialog::~PasswordChangeDialog() = default;