// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageSetSubaddressLookahead.h"
#include "ui_PageSetSubaddressLookahead.h"
#include "WalletWizard.h"

#include <QIntValidator>

PageSetSubaddressLookahead::PageSetSubaddressLookahead(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageSetSubaddressLookahead)
        , m_fields(fields)
{
    ui->setupUi(this);

    // uint32_t can go up to 4294967294, but this isn't realistic
    auto indexValidator = new QIntValidator(1, 2147483647, this);

    ui->line_major->setValidator(indexValidator);
    ui->line_minor->setValidator(indexValidator);

    this->setTitle("Subaddress Lookahead");
}

void PageSetSubaddressLookahead::initializePage() {
    if (m_fields->mode == WizardMode::CreateWalletFromDevice) {
        ui->line_major->setText("5");
        ui->line_minor->setText("20");
    } else {
        ui->line_major->setText("50");
        ui->line_minor->setText("200");
    }
}

bool PageSetSubaddressLookahead::validatePage() {
    m_fields->subaddressLookahead = QString("%1:%2").arg(ui->line_major->text(), ui->line_minor->text());
    return true;
}

int PageSetSubaddressLookahead::nextId() const {
    return WalletWizard::Page_WalletFile;
}