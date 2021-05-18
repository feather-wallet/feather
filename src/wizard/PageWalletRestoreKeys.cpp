// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageWalletRestoreKeys.h"
#include "ui_PageWalletRestoreKeys.h"

#include <QPlainTextEdit>

#include "WalletWizard.h"
#include "constants.h"

PageWalletRestoreKeys::PageWalletRestoreKeys(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletRestoreKeys)
    , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Restore wallet from keys");
    ui->label_errorString->hide();

    QPixmap pixmap = QPixmap(":/assets/images/key.png");
    ui->icon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::restoreOverrideCursor();
#endif

    if (constants::networkType == NetworkType::Type::MAINNET) {
        ui->line_address->setPlaceholderText("4...");
    } else if (constants::networkType == NetworkType::Type::STAGENET) {
        ui->line_address->setPlaceholderText("5...");
    }
}

void PageWalletRestoreKeys::initializePage() {
    ui->line_address->setText("");
    ui->line_viewkey->setText("");
    ui->line_spendkey->setText("");
}

int PageWalletRestoreKeys::nextId() const {
    return WalletWizard::Page_SetRestoreHeight;
}

bool PageWalletRestoreKeys::validatePage() {
    auto errStyle = "QLineEdit{border: 1px solid red;}";

    ui->line_address->setStyleSheet("");
    ui->line_viewkey->setStyleSheet("");
    ui->label_errorString->hide();

    QString address = ui->line_address->text().trimmed();
    QString viewkey = ui->line_viewkey->text().trimmed();
    QString spendkey = ui->line_spendkey->text().trimmed();

    if(!WalletManager::addressValid(address, constants::networkType)){
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid address.");
        ui->line_address->setStyleSheet(errStyle);
        return false;
    }

    if(!WalletManager::keyValid(viewkey, address, true, constants::networkType)) {
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid key.");
        ui->line_viewkey->setStyleSheet(errStyle);
        return false;
    }

    if(!spendkey.isEmpty() && !WalletManager::keyValid(spendkey, address, false, constants::networkType)) {
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid key.");
        ui->line_viewkey->setStyleSheet(errStyle);
        return false;
    }

    m_fields->address = address;
    m_fields->secretViewKey = viewkey;
    m_fields->secretSpendKey = spendkey;
    return true;
}
