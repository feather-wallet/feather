// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "wizard/createwalletseed.h"
#include "wizard/walletwizard.h"
#include "ui_createwalletseed.h"

#include <QFileDialog>
#include <QMessageBox>

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

    ui->seedWord2->setHelpText("In addition to the private spend key, Tevador's 14 word seed scheme also encodes the "
                               "restore date, cryptocurrency type, and reserves a few bits for future use. "
                               "The second word is static because the reserved bits remain the same for each seed generation.");

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
    FeatherSeed seed = FeatherSeed(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName, m_ctx->seedLanguage);
    m_mnemonic = seed.mnemonic.join(" ");
    m_restoreHeight = seed.restoreHeight;

    this->displaySeed(m_mnemonic);
    QTimer::singleShot(10, [=] {
        this->seedRoulette(count);
    });
}

void CreateWalletSeedPage::displaySeed(const QString &seed){
    QStringList seedSplit = seed.split(" ");

    ui->seedWord1->setText(seedSplit[0]);
    ui->seedWord2->setText(seedSplit[1]);
    ui->seedWord3->setText(seedSplit[2]);
    ui->seedWord4->setText(seedSplit[3]);
    ui->seedWord5->setText(seedSplit[4]);
    ui->seedWord6->setText(seedSplit[5]);
    ui->seedWord7->setText(seedSplit[6]);
    ui->seedWord8->setText(seedSplit[7]);
    ui->seedWord9->setText(seedSplit[8]);
    ui->seedWord10->setText(seedSplit[9]);
    ui->seedWord11->setText(seedSplit[10]);
    ui->seedWord12->setText(seedSplit[11]);
    ui->seedWord13->setText(seedSplit[12]);
    ui->seedWord14->setText(seedSplit[13]);
}

int CreateWalletSeedPage::nextId() const {
    return -1;
}

bool CreateWalletSeedPage::validatePage() {
    if(m_mnemonic.isEmpty()) return false;
    if(!m_restoreHeight) return false;

    QMessageBox seedWarning(this);
    seedWarning.setWindowTitle("Warning!");
    seedWarning.setText("• Never disclose your seed\n"
                        "• Never type it on a website\n"
                        "• Store it safely (offline)\n"
                        "• Do not lose your seed!");
    seedWarning.addButton("I understand", QMessageBox::AcceptRole);
    seedWarning.exec();

    this->setField("mnemonicSeed", m_mnemonic);
    this->setField("restoreHeight", m_restoreHeight);
    emit createWallet();
    return true;
}