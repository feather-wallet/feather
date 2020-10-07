// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "passwordchangedialog.h"
#include "ui_passwordchangedialog.h"

#include <QPushButton>

PasswordChangeDialog::PasswordChangeDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::PasswordChangeDialog)
{
    ui->setupUi(this);
    ui->icon->setPixmap(QPixmap(":/assets/images/lock.png").scaledToWidth(32, Qt::SmoothTransformation));

    connect(ui->lineEdit_newPassword, &QLineEdit::textChanged, this, &PasswordChangeDialog::passwordsMatch);
    connect(ui->lineEdit_confirmPassword, &QLineEdit::textChanged, this, &PasswordChangeDialog::passwordsMatch);

    this->adjustSize();
}

PasswordChangeDialog::~PasswordChangeDialog()
{
    delete ui;
}

QString PasswordChangeDialog::getCurrentPassword() {
    return ui->lineEdit_currentPassword->text();
}

QString PasswordChangeDialog::getNewPassword() {
    return ui->lineEdit_newPassword->text();
}

void PasswordChangeDialog::passwordsMatch() {
    bool match = ui->lineEdit_newPassword->text() == ui->lineEdit_confirmPassword->text();
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(match);
}