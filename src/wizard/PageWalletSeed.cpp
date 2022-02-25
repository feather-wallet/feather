// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "WalletWizard.h"
#include "PageWalletSeed.h"
#include "ui_PageWalletSeed.h"
#include "constants.h"
#include "Seed.h"

#include <QMessageBox>

PageWalletSeed::PageWalletSeed(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletSeed)
    , m_fields(fields)
{
    ui->setupUi(this);

    ui->frame_invalidSeed->hide();
    QPixmap warningIcon = QPixmap(":/assets/images/warning.png");
    ui->warningIcon->setPixmap(warningIcon.scaledToWidth(32, Qt::SmoothTransformation));

    QPixmap pixmap = QPixmap(":/assets/images/seed.png");
    ui->seedIcon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    ui->seedWord2->setHelpText("In addition to the private spend key, Tevador's 14 word seed scheme also encodes the "
                               "restore date, cryptocurrency type, and reserves a few bits for future use. "
                               "The second word is static because the reserved bits remain the same for each seed generation.");

    connect(ui->btnRoulette, &QPushButton::clicked, [=]{
        this->seedRoulette(0);
    });
    connect(ui->btnCopy, &QPushButton::clicked, [this]{
        Utils::copyToClipboard(m_seed.mnemonic.join(" "));
    });
}

void PageWalletSeed::initializePage() {
    this->generateSeed();
    this->setTitle(m_fields->modeText);
}

void PageWalletSeed::seedRoulette(int count) {
    count += 1;
    if (count > m_rouletteSpin)
        return;

    this->generateSeed();

    QTimer::singleShot(10, [=] {
        this->seedRoulette(count);
    });
}

void PageWalletSeed::generateSeed() {
    QString mnemonic;

    do {
        m_seed = Seed(Seed::Type::TEVADOR);
        mnemonic = m_seed.mnemonic.join(" ");
        m_restoreHeight = m_seed.restoreHeight;
    } while (mnemonic.split(" ").length() != 14); // https://github.com/tevador/monero-seed/issues/2

    this->displaySeed(mnemonic);

    if (!m_seed.errorString.isEmpty()) {
        ui->frame_invalidSeed->show();
        ui->frame_seedDisplay->hide();
        m_seedError = true;
    }
}

void PageWalletSeed::displaySeed(const QString &seed){
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

int PageWalletSeed::nextId() const {
    return WalletWizard::Page_WalletFile;
}

bool PageWalletSeed::validatePage() {
    if (m_seed.mnemonic.isEmpty()) return false;
    if (!m_restoreHeight) return false;

    QMessageBox seedWarning(this);
    seedWarning.setWindowTitle("Warning!");
    seedWarning.setText("• Never disclose your seed\n"
                        "• Never type it on a website\n"
                        "• Store it safely (offline)\n"
                        "• Do not lose your seed!");
    seedWarning.addButton("Go back", QMessageBox::RejectRole);
    seedWarning.addButton("I understand", QMessageBox::AcceptRole);
    int res = seedWarning.exec();

    if (res == QMessageBox::Rejected) {
        return false;
    }

    m_fields->seed = m_seed;

    return true;
}

bool PageWalletSeed::isComplete() const {
    return !m_seedError;
}