// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "WalletWizard.h"
#include "PageWalletRestoreSeed.h"
#include "ui_PageWalletRestoreSeed.h"

#include <QPlainTextEdit>
#include <QMessageBox>

#include <monero_seed/wordlist.hpp>  // tevador 14 word
#include "utils/Seed.h"
#include "constants.h"

#include <mnemonics/electrum-words.h>

PageWalletRestoreSeed::PageWalletRestoreSeed(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletRestoreSeed)
    , m_fields(fields)
{
    ui->setupUi(this);

    std::vector<const Language::Base*> wordlists = crypto::ElectrumWords::get_language_list();
    for (const auto& wordlist: wordlists) {
        QStringList words_qt;
        std::vector<std::string> words_std = wordlist->get_word_list();
        for (const auto& word: words_std) {
            words_qt += QString::fromStdString(word);
        }

        QString language = QString::fromStdString(wordlist->get_english_language_name());
        ui->combo_seedLanguage->addItem(language);
        m_wordlists[language] = words_qt;
    }

    QStringList bip39English;
    for (int i = 0; i != 2048; i++)
        bip39English << QString::fromStdString(wordlist::english.get_word(i));

    m_polyseed.length = 16;
    m_polyseed.setWords(bip39English);

    // Restore has limited error correction capability, namely it can correct a single erasure
    // (illegible word with a known location). This can be tested by replacing a word with xxxx
    bip39English << "xxxx";

    m_tevador.length = 14;
    m_tevador.setWords(bip39English);

    m_legacy.length = 25;
    m_legacy.setWords(m_wordlists["English"]);
    ui->combo_seedLanguage->setCurrentText("English");

    ui->seedEdit->setAcceptRichText(false);
    ui->seedEdit->setMaximumHeight(150);

    connect(ui->seedBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &PageWalletRestoreSeed::onSeedTypeToggled);
    connect(ui->combo_seedLanguage, &QComboBox::currentTextChanged, this, &PageWalletRestoreSeed::onSeedLanguageChanged);

    this->onSeedTypeToggled();
}

void PageWalletRestoreSeed::onSeedTypeToggled() {
    if (ui->radio16->isChecked()) {
        m_mode = &m_polyseed;
        m_fields->seedType = Seed::Type::POLYSEED;
        ui->seedEdit->setPlaceholderText("Enter 16 word seed..");
        ui->group_seedLanguage->hide();
    }
    if (ui->radio14->isChecked()) {
        m_mode = &m_tevador;
        m_fields->seedType = Seed::Type::TEVADOR;
        ui->seedEdit->setPlaceholderText("Enter 14 word seed..");
        ui->group_seedLanguage->hide();
    }
    else if (ui->radio25->isChecked()) {
        m_mode = &m_legacy;
        m_fields->seedType = Seed::Type::MONERO;
        ui->seedEdit->setPlaceholderText("Enter 25 word seed..");
        ui->group_seedLanguage->show();
    }

    ui->label_errorString->hide();
    ui->seedEdit->setStyleSheet("");
    ui->seedEdit->setCompleter(&m_mode->completer);
    ui->seedEdit->setText("");
}

void PageWalletRestoreSeed::onSeedLanguageChanged(const QString &language) {
    m_legacy.setWords(m_wordlists[language]);
    m_fields->seedLanguage = language;
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
    auto seed = ui->seedEdit->toPlainText().replace("\n", " ").replace("\r", "").trimmed();
    QStringList seedSplit = seed.split(" ", Qt::SkipEmptyParts);

    if (seedSplit.length() != m_mode->length) {
        ui->label_errorString->show();
        ui->label_errorString->setText(QString("The mnemonic seed should be %1 words.").arg(m_mode->length));
        ui->seedEdit->setStyleSheet(errStyle);
        return false;
    }

    // libwallet will accept e.g. "brötchen" or "BRÖTCHEN" instead of "Brötchen"
    QStringList lowercaseWords;
    for (const auto &word : m_mode->words) {
        lowercaseWords << word.toLower();
    }

    for (const auto &word : seedSplit) {
        if (!lowercaseWords.contains(word.toLower())) {
            ui->label_errorString->show();
            ui->label_errorString->setText(QString("Mnemonic seed contains an unknown word: %1").arg(word));
            ui->seedEdit->setStyleSheet(errStyle);
            return false;
        }
    }

    Seed _seed = Seed(m_fields->seedType, seedSplit, constants::networkType);

    if (!_seed.errorString.isEmpty()) {
        QMessageBox::warning(this, "Invalid seed", QString("Invalid seed:\n\n%1").arg(_seed.errorString));
        ui->seedEdit->setStyleSheet(errStyle);
        return false;
    }
    if (!_seed.correction.isEmpty()) {
        QMessageBox::information(this, "Corrected erasure", QString("xxxx -> %1").arg(_seed.correction));
    }

    m_fields->seed = _seed;
    m_fields->seedOffsetPassphrase = ui->line_seedOffset->text();

    return true;
}
