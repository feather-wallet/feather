// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "SeedRecoveryDialog.h"
#include "ui_SeedRecoveryDialog.h"

#include <monero_seed/wordlist.hpp>
#include "ColorScheme.h"
#include "utils/Utils.h"
#include "polyseed/polyseed.h"
#include "utils/AsyncTask.h"
#include "device/device_default.hpp"
#include "cryptonote_basic/account.h"
#include "cryptonote_basic/cryptonote_basic_impl.h"

SeedRecoveryDialog::SeedRecoveryDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , m_scheduler(this)
        , m_watcher(this)
        , ui(new Ui::SeedRecoveryDialog)
{
    ui->setupUi(this);

    for (int i = 0; i != 2048; i++) {
        m_wordList << QString::fromStdString(wordlist::english.get_word(i));
    }

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setText("Check");

    connect(this, &SeedRecoveryDialog::progressUpdated, this, &SeedRecoveryDialog::onProgressUpdated);

    disconnect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &SeedRecoveryDialog::checkSeed);
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, [this]{
        m_cancelled = true;
    });
    connect(ui->buttonBox->button(QDialogButtonBox::Close), &QPushButton::clicked, [this]{
        m_cancelled = true;
        m_watcher.waitForFinished();
        this->close();
    });

    connect(this, &SeedRecoveryDialog::searchFinished, this, &SeedRecoveryDialog::onFinished);
    connect(this, &SeedRecoveryDialog::matchFound, this, &SeedRecoveryDialog::onMatchFound);
    connect(this, &SeedRecoveryDialog::addressMatchFound, this, &SeedRecoveryDialog::onAddressMatchFound);

    this->adjustSize();
}

void SeedRecoveryDialog::onMatchFound(const QString &match) {
    ui->potentialSeeds->appendPlainText(match);
}

void SeedRecoveryDialog::onAddressMatchFound(const QString &match) {
    ui->potentialSeeds->appendPlainText(QString("\nFound seed containing address:\n%1").arg(match));
}

void SeedRecoveryDialog::onFinished(bool cancelled) {
    if (!cancelled) {
        ui->progressBar->setMaximum(100);
        ui->progressBar->setValue(100);
    }

    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(true);
}

QStringList SeedRecoveryDialog::wordsWithRegex(const QRegularExpression &regex) {
    return m_wordList.filter(regex);
}

bool SeedRecoveryDialog::findNext(const QList<QStringList> &words, QList<int> &index) {
    if (words.length() != index.length()) {
        return false;
    }

    if (words.empty()) {
        return false;
    }

    for (int i = words.length() - 1; i >= 0; i--) {
        if ((words[i].length() - 1) > index[i]) {
            index[i] += 1;
            for (int j = i + 1; j < words.length(); j++) {
                index[j] = 0;
            }
            return true;
        }
    }

    return false;
}

QString SeedRecoveryDialog::mnemonic(const QList<QStringList> &words, const QList<int> &index) {
    if (words.length() != index.length()) {
        return QString();
    }

    QStringList mnemonic;
    for (int i = 0; i < words.length(); i++) {
        mnemonic.push_back(words[i][index[i]]);
    }

    return mnemonic.join(" ");
}

bool SeedRecoveryDialog::isAlpha(const QString &word) {
    for (const QChar &ch : word) {
        if (!ch.isLetter()) {
            return false;
        }
    }
    return true;
}

void SeedRecoveryDialog::onProgressUpdated(int value) {
    ui->progressBar->setValue(value);
}

void SeedRecoveryDialog::checkSeed() {
    m_cancelled = false;

    ui->progressBar->setValue(0);
    ui->potentialSeeds->clear();

    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setEnabled(true);

    // Check address
    QString address = ui->line_address->text();
    crypto::public_key spkey = crypto::null_pkey;

    if (!address.isEmpty()) {
        cryptonote::address_parse_info info;
        bool addressValid = cryptonote::get_account_address_from_str(info, cryptonote::network_type::MAINNET, address.toStdString());
        if (!addressValid) {
            Utils::showError(this, "Invalid address entered");
            this->onFinished(false);
            return;
        }
        spkey = info.address.m_spend_public_key;
    }

    QList<QLineEdit*> lineEdits = ui->group_seed->findChildren<QLineEdit*>();
    std::sort(lineEdits.begin(), lineEdits.end(), [](QLineEdit* a, QLineEdit* b) {
        return a->objectName() < b->objectName();
    });

    QList<QStringList> words;
    uint64_t combinations = 1;

    for (QLineEdit *lineEdit : lineEdits) {
        lineEdit->setStyleSheet("");
    }

    for (QLineEdit *lineEdit : lineEdits) {
        ColorScheme::updateFromWidget(this);
        QString word = lineEdit->text();

        QString wordRe = word;
        if (this->isAlpha(word)) {
            wordRe = QString("^%1").arg(wordRe);
        }
        QRegularExpression regex{wordRe};

        if (!regex.isValid()) {
            lineEdit->setStyleSheet(ColorScheme::RED.asStylesheet(true));
            Utils::showError(this, "Invalid regex entered", QString("'%1' is not a valid regular expression").arg(wordRe));
            this->onFinished(false);
            return;
        }

        QStringList possibleWords = wordsWithRegex(regex);
        int numWords = possibleWords.length();

        if (numWords == 1) {
            lineEdit->setStyleSheet(ColorScheme::GREEN.asStylesheet(true));
        }
        else if (numWords == 0) {
            lineEdit->setStyleSheet(ColorScheme::RED.asStylesheet(true));
            Utils::showError(this, "Word is not in wordlist", QString("No words found for: '%1'").arg(word));
            this->onFinished(false);
            return;
        } else {
            lineEdit->setStyleSheet(ColorScheme::YELLOW.asStylesheet(true));
            ui->potentialSeeds->appendPlainText(QString("Possible words for '%1': %2").arg(word, possibleWords.join(", ")));

            if (combinations < std::numeric_limits<uint64_t>::max() / numWords) {
                combinations *= possibleWords.length();
            } else {
                Utils::showError(this, "Too many possible seeds", "Recovery infeasible");
                this->onFinished(false);
                return;
            }
        }

        words << possibleWords;
    }

    if (spkey == crypto::null_pkey) {
        ui->potentialSeeds->appendPlainText("\nPossible seeds:");
    }

    qDebug() << "Number of possible combinations: " << combinations;

    ui->progressBar->setMaximum(combinations / 1000);

    uint32_t major = ui->line_majorLookahead->text().toInt();
    uint32_t minor = ui->line_minorLookahead->text().toInt();

    // Single threaded for now
    const auto future = m_scheduler.run([this, words, spkey, major, minor]{
        QList<int> index(16, 0);

        qint64 i = 0;

        do {
            if (m_cancelled) {
                emit searchFinished(true);
                return;
            }

            if (++i % 1000 == 0) {
                emit progressUpdated(i / 1000);
            }

            QString seedString = mnemonic(words, index);

            crypto::secret_key key;
            try {
                polyseed::data seed(POLYSEED_MONERO);
                seed.decode(seedString.toStdString().c_str());
                seed.keygen(&key.data, sizeof(key.data));
            }
            catch (const polyseed::error& ex) {
                continue;
            }

            // Handle case where we don't know an address
            if (spkey == crypto::null_pkey) {
                emit matchFound(seedString);
                continue;
            }

            cryptonote::account_base base;
            base.generate(key, true, false);

            hw::device &hwdev = base.get_device();

            for (int x = 0; x < major; x++) {
                const std::vector<crypto::public_key> pkeys = hwdev.get_subaddress_spend_public_keys(base.get_keys(), x, 0, minor);
                for (const auto &k : pkeys) {
                    if (k == spkey) {
                        emit addressMatchFound(seedString);
                        emit searchFinished(false);
                        return;
                    }
                }
            }
        } while (findNext(words, index));

        emit searchFinished(false);
    });

    m_watcher.setFuture(future.second);
}

SeedRecoveryDialog::~SeedRecoveryDialog() = default;