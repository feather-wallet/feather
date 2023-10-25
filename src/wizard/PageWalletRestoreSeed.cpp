// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WalletWizard.h"
#include "PageWalletRestoreSeed.h"
#include "ui_PageWalletRestoreSeed.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QShortcut>

#include "dialog/SeedRecoveryDialog.h"
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

    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+K"), this);
    QObject::connect(shortcut, &QShortcut::activated, [&](){
        SeedRecoveryDialog dialog{this};
        dialog.exec();
    });

    ui->seedObscured->hide();
    connect(ui->check_obscureSeed, &QPushButton::clicked, [this](bool checked){
        ui->seedEdit->setVisible(!checked);
        ui->seedObscured->setVisible(checked);
        if (checked) {
            ui->seedObscured->setText(ui->seedEdit->toPlainText());
        } else {
            ui->seedEdit->setText(ui->seedObscured->text());
        }
    });

    connect(ui->seedBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &PageWalletRestoreSeed::onSeedTypeToggled);
    connect(ui->combo_seedLanguage, &QComboBox::currentTextChanged, this, &PageWalletRestoreSeed::onSeedLanguageChanged);
    connect(ui->btnOptions, &QPushButton::clicked, this, &PageWalletRestoreSeed::onOptionsClicked);
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
}

void PageWalletRestoreSeed::onSeedLanguageChanged(const QString &language) {
    m_legacy.setWords(m_wordlists[language]);
    m_fields->seedLanguage = language;
}

int PageWalletRestoreSeed::nextId() const {
    if (m_mode == &m_legacy || m_fields->showSetRestoreHeightPage) {
        return WalletWizard::Page_SetRestoreHeight;
    }

    if (m_fields->showSetSeedPassphrasePage) {
        return WalletWizard::Page_SetSeedPassphrase;
    }

    if (m_fields->showSetSubaddressLookaheadPage) {
        return WalletWizard::Page_SetSubaddressLookahead;
    }

    return WalletWizard::Page_WalletFile;
}

void PageWalletRestoreSeed::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->seedObscured->setText("");
    ui->seedEdit->setText("");
    ui->seedEdit->setStyleSheet("");
    ui->label_errorString->hide();
    ui->radio16->isChecked();
    this->onSeedTypeToggled();
}

bool PageWalletRestoreSeed::validatePage() {
    ui->label_errorString->hide();
    ui->seedEdit->setStyleSheet("");

    QString seed = [this]{
        if (ui->check_obscureSeed->isChecked()) {
            return ui->seedObscured->text();
        } else {
            return ui->seedEdit->toPlainText();
        }
    }();

    seed = seed.replace("\n", " ").replace("\r", "").trimmed();

    auto errStyle = "QTextEdit{border: 1px solid red;}";
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

    if (_seed.encrypted) {
        Utils::showError(this, "Encrypted seed", "This seed is encrypted. Encrypted seeds are not supported");
        return false;
    }

    if (!_seed.errorString.isEmpty()) {
        Utils::showError(this, "Invalid seed", _seed.errorString);
        ui->seedEdit->setStyleSheet(errStyle);
        return false;
    }
    if (!_seed.correction.isEmpty()) {
        Utils::showInfo(this, "Corrected erasure", QString("xxxx -> %1").arg(_seed.correction));
    }

    m_fields->seed = _seed;

    return true;
}

void PageWalletRestoreSeed::onOptionsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Options");

    QVBoxLayout layout;
    QCheckBox check_overrideCreationDate("Override embedded wallet creation date");
    check_overrideCreationDate.setChecked(m_fields->showSetRestoreHeightPage);

    QCheckBox check_setSeedPasshprase("Extend this seed with a passphrase");
    check_setSeedPasshprase.setChecked(m_fields->showSetSeedPassphrasePage);

    QCheckBox check_subaddressLookahead("Set subaddress lookahead");
    check_subaddressLookahead.setChecked(m_fields->showSetSubaddressLookaheadPage);

    layout.addWidget(&check_overrideCreationDate);
    layout.addWidget(&check_setSeedPasshprase);
    layout.addWidget(&check_subaddressLookahead);
    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();

    m_fields->showSetRestoreHeightPage = check_overrideCreationDate.isChecked();
    m_fields->showSetSeedPassphrasePage = check_setSeedPasshprase.isChecked();
    m_fields->showSetSubaddressLookaheadPage = check_subaddressLookahead.isChecked();
}