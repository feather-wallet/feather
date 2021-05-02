// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WalletWizard.h"
#include "PageWalletRestoreSeed.h"
#include "ui_PageWalletRestoreSeed.h"

#include <QPlainTextEdit>
#include <QMessageBox>

#include <monero_seed/wordlist.hpp>  // tevador 14 word
#include "utils/FeatherSeed.h"
#include "globals.h"

PageWalletRestoreSeed::PageWalletRestoreSeed(AppContext *ctx, WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletRestoreSeed)
    , m_ctx(ctx)
    , m_fields(fields)
{
    ui->setupUi(this);
    ui->label_errorString->hide();

    QStringList bip39English;
    for (int i = 0; i != 2048; i++)
        bip39English << QString::fromStdString(wordlist::english.get_word(i));
    // Restore has limited error correction capability, namely it can correct a single erasure
    // (illegible word with a known location). This can be tested by replacing a word with xxxx
    bip39English << "xxxx";

    QByteArray data = Utils::fileOpen(":/assets/mnemonic_25_english.txt");
    QStringList moneroEnglish;
    for (const auto &seed_word: data.split('\n'))
        moneroEnglish << seed_word;

    m_tevador.length = 14;
    m_tevador.setWords(bip39English);

    m_legacy.length = 25;
    m_legacy.setWords(moneroEnglish);

    ui->seedEdit->setAcceptRichText(false);
    ui->seedEdit->setMaximumHeight(150);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::restoreOverrideCursor();
#endif

    connect(ui->seedBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &PageWalletRestoreSeed::onSeedTypeToggled);

    this->onSeedTypeToggled();
}

void PageWalletRestoreSeed::onSeedTypeToggled() {
    if (ui->radio14->isChecked()) {
        m_mode = &m_tevador;
        m_fields->seedType = SeedType::TEVADOR;
        ui->seedEdit->setPlaceholderText("Enter 14 word seed..");
    }
    else if (ui->radio25->isChecked()) {
        m_mode = &m_legacy;
        m_fields->seedType = SeedType::MONERO;
        ui->seedEdit->setPlaceholderText("Enter 25 word seed..");
    }

    ui->label_errorString->hide();
    ui->seedEdit->setStyleSheet("");
    ui->seedEdit->setCompleter(&m_mode->completer);
    ui->seedEdit->setText("");
}

int PageWalletRestoreSeed::nextId() const {
    if (m_mode == &m_legacy) {
        return WalletWizard::Page_SetRestoreHeight;
    }

    return WalletWizard::Page_WalletFile;
}

void PageWalletRestoreSeed::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->seedEdit->setText("");
    ui->seedEdit->setStyleSheet("");
    ui->label_errorString->hide();
    ui->line_seedOffset->setText("");
}

bool PageWalletRestoreSeed::validatePage() {
    ui->label_errorString->hide();
    ui->seedEdit->setStyleSheet("");

    auto errStyle = "QTextEdit{border: 1px solid red;}";
    auto seed = ui->seedEdit->toPlainText().replace("\n", "").replace("\r", "").trimmed();
    auto seedSplit = seed.split(" ");

    if (seedSplit.length() != m_mode->length) {
        ui->label_errorString->show();
        ui->label_errorString->setText(QString("The mnemonic seed should be %1 words.").arg(m_mode->length));
        ui->seedEdit->setStyleSheet(errStyle);
        return false;
    }

    for (const auto &word : seedSplit) {
        if (!m_mode->words.contains(word)) {
            ui->label_errorString->show();
            ui->label_errorString->setText(QString("Mnemonic seed contains an unknown word: %1").arg(word));
            ui->seedEdit->setStyleSheet(errStyle);
            return false;
        }
    }

    auto _seed = FeatherSeed(m_ctx->networkType, QString::fromStdString(globals::coinName), m_ctx->seedLanguage, seedSplit);
    if (!_seed.errorString.isEmpty()) {
        QMessageBox::warning(this, "Invalid seed", QString("Invalid seed:\n\n%1").arg(_seed.errorString));
        ui->seedEdit->setStyleSheet(errStyle);
        return false;
    }
    if (!_seed.correction.isEmpty()) {
        QMessageBox::information(this, "Corrected erasure", QString("xxxx -> %1").arg(_seed.correction));
    }

    m_fields->seed = seed;
    m_fields->seedOffsetPassphrase = ui->line_seedOffset->text();

    return true;
}
