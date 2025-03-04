// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SeedDialog.h"

#include "Utils.h"
#include "ui_SeedDialog.h"

#include "constants.h"

SeedDialog::SeedDialog(Wallet *wallet, QWidget *parent)
    : WindowModalDialog(parent)
    , ui(new Ui::SeedDialog)
    , m_wallet(wallet)
{
    ui->setupUi(this);
    ui->label_seedIcon->setPixmap(QPixmap(":/assets/images/seed.png").scaledToWidth(64, Qt::SmoothTransformation));

    ui->label_restoreHeight->setText(Utils::formatRestoreHeight(m_wallet->getWalletCreationHeight()));

    if (m_wallet->getSeedLanguage().isEmpty()) {
        qDebug() << "No seed language set, using default";
        m_wallet->setSeedLanguage(constants::seedLanguage);
    }

    QString seedOffset = m_wallet->getCacheAttribute("feather.seedoffset");
    QString seed = m_wallet->getCacheAttribute("feather.seed");
    auto seedLength = m_wallet->seedLength();

    QString seed_25_words = m_wallet->getSeed(seedOffset);

    if (seedLength >= 24) {
        ui->check_toggleSeedType->hide();
        this->setSeed(seed_25_words);
    } else {
        this->setSeed(seed);
        ui->frameRestoreHeight->setVisible(false);
    }

    ui->frameSeedOffset->setVisible(!seedOffset.isEmpty());
    ui->line_seedOffset->setText(seedOffset);

    connect(ui->check_toggleSeedType, &QCheckBox::toggled, [this, seed_25_words, seed](bool toggled){
        this->setSeed(toggled ? seed_25_words : seed);
        ui->frameRestoreHeight->setVisible(toggled);
    });

    ui->label_restoreHeightHelp->setHelpText("", "Should you restore your wallet in the future, "
                                             "specifying this block number will recover your wallet quicker.", "restore_height");

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

SeedDialog::~SeedDialog() = default;