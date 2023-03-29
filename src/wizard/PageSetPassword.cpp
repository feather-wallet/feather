// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageSetPassword.h"
#include "ui_PageSetPassword.h"
#include "WalletWizard.h"

PageSetPassword::PageSetPassword(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageSetPassword)
    , m_fields(fields)
{
    ui->setupUi(this);
    this->setFinalPage(true);

    QPixmap pixmap = QPixmap(":/assets/images/lock.svg");
    ui->icon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    connect(ui->line_password, &QLineEdit::textChanged, [this]{
        this->completeChanged();
    });
    connect(ui->line_confirmPassword, &QLineEdit::textChanged, [this]{
        this->completeChanged();
    });

    this->setButtonText(QWizard::FinishButton, "Create/Open wallet");
}

void PageSetPassword::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->line_password->setText("");
    ui->line_confirmPassword->setText("");
}

bool PageSetPassword::validatePage() {
    m_fields->password = ui->line_password->text();
    emit createWallet();
    return true;
}

int PageSetPassword::nextId() const {
    return -1;
}

bool PageSetPassword::isComplete() const {
    return ui->line_password->text() == ui->line_confirmPassword->text();
}
