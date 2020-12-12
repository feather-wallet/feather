// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "ui_seeddialog.h"
#include "seeddialog.h"

SeedDialog::SeedDialog(Wallet *wallet, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::SeedDialog)
{
    ui->setupUi(this);
    ui->label_seedIcon->setPixmap(QPixmap(":/assets/images/seed.png").scaledToWidth(64, Qt::SmoothTransformation));

    ui->label_restoreHeight->setText(QString::number(wallet->getWalletCreationHeight()));

    QString seed_14_words = wallet->getCacheAttribute("feather.seed");
    QString seed_25_words = wallet->getSeed();

    if (seed_14_words.isEmpty()) {
        ui->check_toggleSeedType->hide();
        this->setSeed(seed_25_words);
    } else {
        this->setSeed(seed_14_words);
        ui->frameRestoreHeight->setVisible(false);
    }

    connect(ui->check_toggleSeedType, &QCheckBox::toggled, [this, seed_25_words, seed_14_words](bool toggled){
        this->setSeed(toggled ? seed_25_words : seed_14_words);
        ui->frameRestoreHeight->setVisible(toggled);
    });

    ui->label_restoreHeightHelp->setHelpText("Should you restore your wallet in the future, "
                                             "specifying this block number will recover your wallet quicker.");

    this->adjustSize();
}

void SeedDialog::setSeed(const QString &seed) {
    ui->seed->setPlainText(seed);

    int words = seed.split(" ").size();
    ui->label_warning->setText(QString("<p>Please save these %1 words on paper (order is important). "
                                       "This seed will allow you to recover your wallet in case "
                                       "of computer failure."
                                       "</p>"
                                       "<b>WARNING:</b>"
                                       "<ul>"
                                       "<li>Never disclose your seed.</li>"
                                       "<li>Never type it on a website</li>"
                                       "<li>Do not store it electronically</li>"
                                       "</ul>").arg(words));
}

SeedDialog::~SeedDialog()
{
    delete ui;
}