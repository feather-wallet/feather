// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "wizard/viewonlywallet.h"
#include "wizard/walletwizard.h"
#include "ui_viewonlywallet.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QPushButton>
#include <QButtonGroup>

#include <monero_seed/wordlist.hpp>  // tevador 14 word

#include "libwalletqt/WalletManager.h"

ViewOnlyPage::ViewOnlyPage(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        ui(new Ui::ViewOnlyPage),
        m_ctx(ctx) {
    ui->setupUi(this);
    this->setTitle("Import view only wallet");
    ui->label_errorString->hide();

    QFont f("feather");
    f.setStyleHint(QFont::Monospace);

    auto *viewOnlyViewKeyDummy = new QLineEdit(this);
    viewOnlyViewKeyDummy->setVisible(false);
    auto *viewOnlySpendKeyDummy = new QLineEdit(this);
    viewOnlySpendKeyDummy->setVisible(false);
    auto *viewOnlyAddressDummy = new QLineEdit(this);
    viewOnlyAddressDummy->setVisible(false);
    auto *restoreHeightDummy = new QLineEdit(this);
    restoreHeightDummy->setVisible(false);

    this->registerField("viewOnlySpendKey", viewOnlySpendKeyDummy);
    this->registerField("viewOnlyViewKey", viewOnlyViewKeyDummy);
    this->registerField("viewOnlyAddress", viewOnlyAddressDummy);
    this->registerField("viewOnlyHeight", restoreHeightDummy);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::restoreOverrideCursor();
#endif

    if(m_ctx->networkType == NetworkType::Type::TESTNET) {
        ui->restoreHeightWidget->hideSlider();
    } else {
        // load restoreHeight lookup db
        ui->restoreHeightWidget->initRestoreHeights(m_ctx->restoreHeights[m_ctx->networkType]);
    }

    if(m_ctx->networkType == NetworkType::Type::MAINNET) {
        ui->lineEdit_address->setPlaceholderText("4...");
    } else if (m_ctx->networkType == NetworkType::Type::STAGENET) {
        ui->lineEdit_address->setPlaceholderText("5...");
    }
}

int ViewOnlyPage::nextId() const {
    return WalletWizard::Page_CreateWallet;
}

void ViewOnlyPage::cleanupPage() const {}

bool ViewOnlyPage::validatePage() {
    auto errStyle = "QLineEdit{border: 1px solid red;}";

    ui->lineEdit_address->setStyleSheet("");
    ui->lineEdit_viewkey->setStyleSheet("");
    ui->label_errorString->hide();

    int restoreHeight = ui->restoreHeightWidget->getHeight();
    auto spendkey = ui->lineEdit_spendkey->text().trimmed();
    auto viewkey = ui->lineEdit_viewkey->text().trimmed();
    auto address = ui->lineEdit_address->text().trimmed();

    if(!m_ctx->walletManager->addressValid(address, m_ctx->networkType)){
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid address.");
        ui->lineEdit_address->setStyleSheet(errStyle);
        return false;
    }

    if(!m_ctx->walletManager->keyValid(viewkey, address, true, m_ctx->networkType)) {
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid key.");
        ui->lineEdit_viewkey->setStyleSheet(errStyle);
        return false;
    }

    if(!spendkey.isEmpty() && !m_ctx->walletManager->keyValid(spendkey, address, false, m_ctx->networkType)) {
        ui->label_errorString->show();
        ui->label_errorString->setText("Invalid key.");
        ui->lineEdit_viewkey->setStyleSheet(errStyle);
        return false;
    }

    this->setField("viewOnlyViewKey", viewkey);
    this->setField("viewOnlySpendKey", spendkey);
    this->setField("viewOnlyAddress", address);
    this->setField("viewOnlyHeight", restoreHeight);
    return true;
}
