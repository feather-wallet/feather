// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "utils/utils.h"
#include "wizard/createwallet.h"
#include "wizard/walletwizard.h"
#include "ui_createwallet.h"
#include "appcontext.h"

#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include "libwalletqt/WalletManager.h"

CreateWalletPage::CreateWalletPage(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        ui(new Ui::CreateWalletPage),
        m_ctx(ctx) {
    ui->setupUi(this);
    this->setTitle("Create wallet");
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    // hide ui element, we only need it for registerField
    this->registerField("walletName*", ui->walletName);
    this->registerField("walletDirectory", ui->directory);
    this->registerField("walletPasswd", ui->password);
    this->registerField("walletPath", ui->walletPath);
    ui->walletPath->hide();

    ui->directory->setText(m_ctx->defaultWalletDir);
    m_walletDir = m_ctx->defaultWalletDir;
    connect(ui->btnChange, &QPushButton::clicked, [=] {
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", m_ctx->defaultWalletDir, QFileDialog::ShowDirsOnly);
        if(walletDir.isEmpty()) return;
        ui->directory->setText(walletDir);
    });

    connect(ui->directory, &QLineEdit::textChanged, [=](const QString &data) {
        m_walletDir = data;
        this->validateWidgets();
    });

    connect(ui->walletName, &QLineEdit::textChanged, [=](QString data) {
        this->validateWidgets();
    });
}

bool CreateWalletPage::validateWidgets(){
    ui->walletName->setStyleSheet("");
    ui->directory->setStyleSheet("");
    auto walletPass = ui->password->text();
    auto errStyle = "QLineEdit{border: 1px solid red;}";
    if(m_walletDir.isEmpty()){
        ui->walletName->setStyleSheet(errStyle);
        ui->directory->setStyleSheet(errStyle);
        return false;
    }

    if(!Utils::dirExists(m_walletDir)) {
        ui->walletName->setStyleSheet(errStyle);
        ui->directory->setStyleSheet(errStyle);
        return false;
    }

    ui->directory->setStyleSheet("");
    auto walletName = ui->walletName->text().replace(".keys", "");
    if(walletName.isEmpty()) {
        ui->walletName->setStyleSheet(errStyle);
        return false;
    }

    auto walletPath = QDir(m_walletDir).filePath(walletName + ".keys");
    if(Utils::fileExists(walletPath)) {
        ui->walletName->setStyleSheet(errStyle);
        return false;
    }

    return true;
}

int CreateWalletPage::nextId() const {
    auto restoredSeed = this->field("mnemonicRestoredSeed").toString();
    auto restoredViewOnlyKey = this->field("viewOnlyViewKey").toString();

    if(!restoredSeed.isEmpty() || !restoredViewOnlyKey.isEmpty())
        return -1;

    return WalletWizard::Page_CreateWalletSeed;
}

bool CreateWalletPage::validatePage() {
    if(!this->validateWidgets()) return false;
    auto walletName = ui->walletName->text().replace(".keys", "");
    auto walletPath = QDir(m_walletDir).filePath(walletName + ".keys");
    this->setField("walletPath", walletPath);
    ui->walletName->setStyleSheet("");

    auto restoredSeed = this->field("mnemonicRestoredSeed").toString();
    auto restoredViewOnlyKey = this->field("viewOnlyViewKey").toString();
    if(!restoredSeed.isEmpty() || !restoredViewOnlyKey.isEmpty()) emit createWallet();
    return true;
}
