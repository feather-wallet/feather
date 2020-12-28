// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "wizard/createwalletseed.h"
#include "wizard/walletwizard.h"
#include "ui_createwalletseed.h"

#include <QFileDialog>

CreateWalletSeedPage::CreateWalletSeedPage(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        m_ctx(ctx),
        ui(new Ui::CreateWalletSeedPage) {
    ui->setupUi(this);
    this->setFinalPage(true);
    this->setTitle("Wallet seed");

    // hide ui element, we only need it for registerField
    this->registerField("mnemonicSeed", ui->hiddenMnemonicSeed);
    ui->hiddenMnemonicSeed->hide();

    ui->seed->setFont(Utils::relativeFont(1));

    connect(ui->btnRoulette, &QPushButton::clicked, [=]{
        this->seedRoulette(0);
    });

    this->setButtonText(QWizard::FinishButton, "Create/Open wallet");

    // generate new seed
    this->seedRoulette(m_rouletteSpin - 1);
}

void CreateWalletSeedPage::seedRoulette(int count) {
    count += 1;
    if(count > m_rouletteSpin) return;
    auto seed = FeatherSeed::generate(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName.toStdString(), m_ctx->seedLanguage);
    m_mnemonic = seed.mnemonicSeed;
    m_restoreHeight = seed.restoreHeight;

    this->displaySeed(m_mnemonic);
    QTimer::singleShot(10, [=] {
        this->seedRoulette(count);
    });
}

void CreateWalletSeedPage::displaySeed(const QString &seed){
    ui->seed->setPlainText(seed);
}

int CreateWalletSeedPage::nextId() const {
    return -1;
}

bool CreateWalletSeedPage::validatePage() {
    if(m_mnemonic.isEmpty()) return false;
    if(!m_restoreHeight) return false;
    this->setField("mnemonicSeed", m_mnemonic);
    this->setField("restoreHeight", m_restoreHeight);
    emit createWallet();
    return true;
}