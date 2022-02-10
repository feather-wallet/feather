// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "WalletWizard.h"
#include "PageWalletSeed.h"
#include "ui_PageWalletSeed.h"
#include "constants.h"

#include <QMessageBox>

PageWalletSeed::PageWalletSeed(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletSeed)
    , m_fields(fields)
{
    ui->setupUi(this);

    QPixmap pixmap = QPixmap(":/assets/images/seed.png");
    ui->seedIcon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    ui->seedWord2->setHelpText("In addition to the private spend key, Tevador's 14 word seed scheme also encodes the "
                               "restore date, cryptocurrency type, and reserves a few bits for future use. "
                               "The second word is static because the reserved bits remain the same for each seed generation.");

    connect(ui->btnRoulette, &QPushButton::clicked, [=]{
        this->seedRoulette(0);
    });
    connect(ui->btnCopy, &QPushButton::clicked, [this]{
        Utils::copyToClipboard(m_mnemonic);
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
    do {
        FeatherSeed seed = FeatherSeed(constants::networkType, QString::fromStdString(constants::coinName), constants::seedLanguage);
        m_mnemonic = seed.mnemonic.join(" ");
        m_restoreHeight = seed.restoreHeight;
    } while (m_mnemonic.split(" ").length() != 14); // https://github.com/tevador/monero-seed/issues/2

    this->displaySeed(m_mnemonic);
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
    if (m_mnemonic.isEmpty()) return false;
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

    m_fields->seed = m_mnemonic;

    return true;
}