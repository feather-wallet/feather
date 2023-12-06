// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WalletWizard.h"
#include "PageWalletSeed.h"
#include "ui_PageWalletSeed.h"

#include <QMessageBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QShortcut>

#include "constants.h"
#include "Seed.h"
#include "Icons.h"
#include "dialog/SeedDiceDialog.h"

PageWalletSeed::PageWalletSeed(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletSeed)
    , m_fields(fields)
{
    ui->setupUi(this);

    ui->frame_notice->setInfo(icons()->icon("seed"), "The following **16** words can be used to recover access to your wallet.\n\n"
                                                   "Write them down and store them somewhere safe and secure.\n\n"
                                                   "Feather uses **Polyseed**. For more information click **Help**.");

    ui->frame_invalidSeed->setInfo(icons()->icon("warning"), "Feather was unable to generate a valid seed.\n"
                                                             "This should never happen.\n"
                                                             "Please contact the developers immediately.");

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    QObject::connect(shortcut, &QShortcut::activated, [&](){
        SeedDiceDialog dialog{this};
        int r = dialog.exec();
        if (r == QDialog::Accepted) {
            if (!dialog.finished()) {
                this->onError();
                Utils::showError(this, "Unable to create polyseed using additional entropy", "Not enough entropy was collected", {"You have found a bug. Please contact the developers."});
                return;
            }

            this->generateSeed(dialog.getSecret());
            dialog.wipeSecret();
            Utils::showInfo(this, "Polyseed created successfully using additional entropy");
        }
    });

    connect(ui->btnRoulette, &QPushButton::clicked, [=]{
        this->seedRoulette(0);
    });
    connect(ui->btnCopy, &QPushButton::clicked, [this]{
        Utils::copyToClipboard(m_seed.mnemonic.join(" "));
    });
    connect(ui->btnOptions, &QPushButton::clicked, this, &PageWalletSeed::onOptionsClicked);
}

void PageWalletSeed::initializePage() {
    ui->frame_invalidSeed->hide();
    ui->frame_seedDisplay->show();

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

void PageWalletSeed::generateSeed(const char* secret) {
    QString mnemonic;

    m_seed = Seed(Seed::Type::POLYSEED, constants::networkType, "English", secret);
    mnemonic = m_seed.mnemonic.join(" ");
    m_restoreHeight = m_seed.restoreHeight;

    this->displaySeed(mnemonic);

    if (!m_seed.errorString.isEmpty()) {
        this->onError();
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
    ui->seedWord15->setText(seedSplit[14]);
    ui->seedWord16->setText(seedSplit[15]);
}

void PageWalletSeed::onOptionsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Options");
    QVBoxLayout layout;
    QCheckBox checkbox("Extend this seed with a passphrase");
    checkbox.setChecked(m_fields->showSetSeedPassphrasePage);
    layout.addWidget(&checkbox);

    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();
    m_fields->showSetSeedPassphrasePage = checkbox.isChecked();
}

void PageWalletSeed::onError() {
    ui->frame_invalidSeed->show();
    ui->frame_seedDisplay->hide();
    m_seedError = true;
    this->completeChanged();
}

int PageWalletSeed::nextId() const {
    if (m_fields->showSetSeedPassphrasePage) {
        return WalletWizard::Page_SetSeedPassphrase;
    }
    return WalletWizard::Page_WalletFile;
}

bool PageWalletSeed::validatePage() {
    if (m_seed.mnemonic.isEmpty()) {
        return false;
    }
    if (!m_restoreHeight) {
        return false;
    }

    QMessageBox seedWarning(this);
    seedWarning.setWindowTitle("Warning!");
    seedWarning.setText("• Never disclose your seed\n"
                        "• Never type it on a website\n"
                        "• Store it safely (offline)\n"
                        "• Do not lose your seed!");
    auto btn_goBack = seedWarning.addButton("Go back", QMessageBox::RejectRole);
    seedWarning.addButton("I understand", QMessageBox::AcceptRole);

    seedWarning.exec();
    if (seedWarning.clickedButton() == btn_goBack) {
        return false;
    }

    m_fields->seed = m_seed;

    return true;
}

bool PageWalletSeed::isComplete() const {
    return !m_seedError;
}