// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PageSetSubaddressLookahead.h"
#include "ui_PageSetSubaddressLookahead.h"
#include "WalletWizard.h"

#include <QRegularExpressionValidator>

#include "Icons.h"

PageSetSubaddressLookahead::PageSetSubaddressLookahead(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageSetSubaddressLookahead)
        , m_fields(fields)
{
    ui->setupUi(this);

    auto *indexValidator = new QRegularExpressionValidator{QRegularExpression("[0-9]{0,5}"), this};

    ui->line_major->setValidator(indexValidator);
    connect(ui->line_major, &QLineEdit::textChanged, [this]{
        this->completeChanged();
    });

    ui->line_minor->setValidator(indexValidator);
    connect(ui->line_major, &QLineEdit::textChanged, [this]{
        this->completeChanged();
    });

    ui->infoFrame->setInfo(icons()->icon("warning"), "Lookahead must be non-zero.");

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
    ui->infoFrame->hide();
}

bool PageSetSubaddressLookahead::validatePage() {
    m_fields->subaddressLookahead = QString("%1:%2").arg(ui->line_major->text(), ui->line_minor->text());
    return true;
}

int PageSetSubaddressLookahead::nextId() const {
    return WalletWizard::Page_WalletFile;
}

bool PageSetSubaddressLookahead::isComplete() const {
    ui->infoFrame->hide();

    if (ui->line_major->text().isEmpty() || ui->line_major->text().toInt() == 0) {
       ui->infoFrame->show();
       return false;
   }
   if (ui->line_minor->text().isEmpty() || ui->line_minor->text().toInt() == 0) {
       ui->infoFrame->show();
       return false;
   }

   return true;
}
