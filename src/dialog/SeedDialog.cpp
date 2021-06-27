// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "ui_SeedDialog.h"
#include "SeedDialog.h"
#include "constants.h"

SeedDialog::SeedDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeedDialog)
    , m_ctx(std::move(ctx))
{
    ui->setupUi(this);
    ui->label_seedIcon->setPixmap(QPixmap(":/assets/images/seed.png").scaledToWidth(64, Qt::SmoothTransformation));

    ui->label_restoreHeight->setText(QString::number(m_ctx->wallet->getWalletCreationHeight()));

    if (m_ctx->wallet->getSeedLanguage().isEmpty()) {
        qDebug() << "No seed language set, using default";
        m_ctx->wallet->setSeedLanguage(constants::seedLanguage);
    }

    QString seedOffset = m_ctx->wallet->getCacheAttribute("feather.seedoffset");
    QString seed_14_words = m_ctx->wallet->getCacheAttribute("feather.seed");
    QString seed_25_words = m_ctx->wallet->getSeed(seedOffset);

    if (seed_14_words.isEmpty()) {
        ui->check_toggleSeedType->hide();
        this->setSeed(seed_25_words);
    } else {
        this->setSeed(seed_14_words);
        ui->frameRestoreHeight->setVisible(false);
    }

    ui->frameSeedOffset->setVisible(!seedOffset.isEmpty());
    ui->line_seedOffset->setText(seedOffset);

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

SeedDialog::~SeedDialog() = default;