// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PasswordSetDialog.h"
#include "ui_PasswordSetDialog.h"

#include "utils/Icons.h"

PasswordSetDialog::PasswordSetDialog(const QString &helpText, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::PasswordSetDialog)
{
    ui->setupUi(this);

    ui->frame_info->setInfo(icons()->icon("lock"), helpText);

    connect(ui->widget_password, &PasswordSetWidget::passwordEntryChanged, [this]{
        bool passwordsMatch = ui->widget_password->passwordsMatch();

        QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
        if (okButton) {
            okButton->setEnabled(passwordsMatch);
        }
    });

    this->adjustSize();
}

QString PasswordSetDialog::password() {
    return ui->widget_password->password();
}

PasswordSetDialog::~PasswordSetDialog() = default;