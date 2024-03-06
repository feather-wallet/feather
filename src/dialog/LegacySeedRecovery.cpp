// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "LegacySeedRecovery.h"
#include "ui_LegacySeedRecovery.h"

#include <mnemonics/electrum-words.h>
#include "ColorScheme.h"
#include "utils/Utils.h"
#include "polyseed/polyseed.h"
#include "utils/AsyncTask.h"
#include "device/device_default.hpp"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"
#include "cryptonote_basic/blobdatatype.h"
#include "common/base58.h"
#include "serialization/binary_utils.h"

LegacySeedRecovery::LegacySeedRecovery(QWidget *parent)
        : WindowModalDialog(parent)
        , m_scheduler(this)
        , m_watcher(this)
        , ui(new Ui::LegacySeedRecovery)
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
        m_wordLists[language] = words_qt;
    }

    ui->combo_seedLanguage->setCurrentIndex(1);

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText("Check");

    disconnect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &LegacySeedRecovery::checkSeed);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, [this]{
        m_cancelled = true;
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, [this]{
        m_cancelled = true;
        m_watcher.waitForFinished();
        this->close();
    });

    connect(this, &LegacySeedRecovery::progressUpdated, this, &LegacySeedRecovery::onProgressUpdated);

    connect(this, &LegacySeedRecovery::searchFinished, this, &LegacySeedRecovery::onFinished);
    connect(this, &LegacySeedRecovery::matchFound, this, &LegacySeedRecovery::onMatchFound);
    connect(this, &LegacySeedRecovery::addressMatchFound, this, &LegacySeedRecovery::onAddressMatchFound);
    connect(this, &LegacySeedRecovery::addResultText, this, &LegacySeedRecovery::onAddResultText);

    this->adjustSize();
}

void LegacySeedRecovery::onMatchFound(const QString &match) {
    ui->results->appendPlainText(match);
}

void LegacySeedRecovery::onAddressMatchFound(const QString &match) {
    ui->results->appendPlainText(QString("Found seed containing address:\n%1").arg(match));
}

void LegacySeedRecovery::onFinished(bool cancelled) {
    if (!cancelled) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
    }

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

void LegacySeedRecovery::onProgressUpdated(int value) {
    ui->progressBar->setValue(value);
}

void LegacySeedRecovery::onAddResultText(const QString &text) {
    ui->results->appendPlainText(text);
}

bool LegacySeedRecovery::testSeed(const QString &seed, const crypto::public_key &spkey) {
    std::string mnemonic = seed.toStdString();

    crypto::secret_key k;
    std::string lang;
    bool r = crypto::ElectrumWords::words_to_bytes(mnemonic, k, lang);

    if (!r) {
        return false;
    }

    if (spkey == crypto::null_pkey) {
        emit matchFound(seed);
        return false;
    }


    cryptonote::account_base base;
    base.generate(k, true, false);

    hw::device &hwdev = base.get_device();

    for (int x = 0; x < m_major; x++) {
        const std::vector<crypto::public_key> pkeys = hwdev.get_subaddress_spend_public_keys(base.get_keys(), x, 0, m_minor);
        for (const auto &k : pkeys) {
            if (k == spkey) {
                emit addressMatchFound(seed);
                emit searchFinished(false);
                return true;
            }
        }
    }

    return false;
}

void LegacySeedRecovery::checkSeed() {
    m_cancelled = false;

    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);

    ui->results->clear();
    ui->progressBar->setMaximum(39024);
    ui->progressBar->setValue(0);

    QStringList words = ui->seed->toPlainText().replace("\n", " ").replace("\r", "").trimmed().split(" ", Qt::SkipEmptyParts);
    if (words.length() < 24) {
        Utils::showError(this, "Invalid seed", "Less than 24 words were entered", {"Remember to use a single space between each word."});
        return;
    }
    if (words.length() > 25) {
        Utils::showError(this, "Invalid seed", "More than 25 words were entered", {"Remember to use a single space between each word."});
        return;
    }

    Mode mode = words.length() == 25 ? Mode::WORD_25 : Mode::WORD_24;

    QString address = ui->line_depositAddress->text();
    crypto::public_key spkey = crypto::null_pkey;

    if (!address.isEmpty()) {
        cryptonote::blobdata data;
        uint64_t prefix;
        if (!tools::base58::decode_addr(address.toStdString(), prefix, data))
        {
            Utils::showError(this, "Unable to decode address");
            this->onFinished(false);
            return;
        }

        cryptonote::account_public_address a;
        if (!::serialization::parse_binary(data, a))
        {
            Utils::showError(this, "Account public address keys can't be parsed");
            this->onFinished(false);
            return;
        }

        if (!crypto::check_key(a.m_spend_public_key) || !crypto::check_key(a.m_view_public_key))
        {
            Utils::showError(this, "Failed to validate address keys");
            this->onFinished(false);
            return;
        }

        spkey = a.m_spend_public_key;
    }

    if (spkey == crypto::null_pkey) {
        ui->results->appendPlainText("\nPossible seeds:");
    }

    m_major = ui->line_majorLookahead->text().toInt();
    m_minor = ui->line_minorLookahead->text().toInt();

    QString language = ui->combo_seedLanguage->currentText();
    if (!m_wordLists.contains(language)) {
        Utils::showError(this, "Unable to start recovery tool", QString("No wordlist for language: %1").arg(language));
        return;
    }

    ui->results->appendPlainText(QString("%1 words entered\n").arg(QString::number(words.length())));

    // Single threaded for now
    const auto future = m_scheduler.run([this, words, spkey, mode, language]{

        if (mode == Mode::WORD_25) {
            emit addResultText("Strategy [1/2]: swap adjacent words\n");

            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(24);

            for (int i = 0; i < 24; i++) {
                QStringList seed = words;
                seed.swapItemsAt(i, i+1);

                QString m = seed.join(" ");
                bool done = this->testSeed(m, spkey);
                if (done) {
                    return;
                }

                // Swap back
                seed.swapItemsAt(i, i+1);
            }

            if (m_cancelled) {
                emit searchFinished(true);
                return;
            }

            emit addResultText("Strategy [2/2]: one word is incorrect\n");

            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(39024);

            int tries = 0;
            for (int i = 0; i < 24; i++) {
                QStringList seed = words;

                for (const auto &word : m_wordLists[language]) {
                    if (m_cancelled) {
                        emit searchFinished(true);
                        return;
                    }
                    emit progressUpdated(++tries);

                    seed[i] = word;

                    QString m = seed.join(" ");
                    bool done = this->testSeed(m, spkey);
                    if (done) {
                        return;
                    }
                }
            }
        }

        if (mode == Mode::WORD_24) {
            emit addResultText("Strategy [1/1]: one word is missing\n");

            ui->progressBar->setValue(0);
            ui->progressBar->setMaximum(39024);

            int tries = 0;
            for (int i = 0; i < 24; i++) {
                QStringList seed = words;
                seed.insert(i, "placeholder");

                for (const auto &word : m_wordLists[language]) {
                    if (m_cancelled) {
                        emit searchFinished(true);
                        return;
                    }
                    emit progressUpdated(++tries);

                    seed[i] = word;
                    QString m = seed.join(" ");

                    bool done = this->testSeed(m, spkey);
                    if (done) {
                        return;
                    }
                }
            }
        }

        emit searchFinished(false);
    });

    m_watcher.setFuture(future.second);
}

LegacySeedRecovery::~LegacySeedRecovery() = default;