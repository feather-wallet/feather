// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "wizard/restorewallet.h"
#include "wizard/walletwizard.h"
#include "ui_restorewallet.h"

#include <QLineEdit>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <QPushButton>
#include <QButtonGroup>

#include <monero_seed/wordlist.hpp>  // tevador 14 word

#include "libwalletqt/WalletManager.h"

RestorePage::RestorePage(AppContext *ctx, QWidget *parent) :
        QWizardPage(parent),
        ui(new Ui::RestorePage),
        m_ctx(ctx) {
    ui->setupUi(this);
    this->setTitle("Restore wallet");
    this->setButtonText(QWizard::FinishButton, "walletKeysFilesModel");
    ui->restoreFrame->hide();
    ui->label_errorString->hide();

    QFont f("feather");
    f.setStyleHint(QFont::Monospace);

    auto data = Utils::fileOpen(":/assets/mnemonic_25_english.txt");
    for(const auto &seed_word: data.split('\n'))
        m_words25 << seed_word;
    for(int i = 0; i != 2048; i++)
        m_words14 << QString::fromStdString(wordlist::english.get_word(i));

    //
    m_completer14Model = new QStringListModel(m_words14, m_completer14);
    m_completer14 = new QCompleter(this);
    m_completer14->setModel(m_completer14Model);
    m_completer14->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer14->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer14->setWrapAround(false);
    //
    m_completer25Model = new QStringListModel(m_words25, m_completer25);
    m_completer25 = new QCompleter(this);
    m_completer25->setModel(m_completer25Model);
    m_completer25->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_completer25->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer25->setWrapAround(false);
    //
    ui->seedEdit->setCompleter(m_completer14);
    ui->seedEdit->setAcceptRichText(false);
    ui->seedEdit->setMaximumHeight(80);
    ui->seedEdit->setFrameShape(QFrame::Box);
    ui->seedEdit->setFrameShadow(QFrame::Plain);
    ui->seedEdit->setFont(f);
    ui->seedEdit->setPlaceholderText("Insert your mnemonic 14 word seed...");
    //

    auto *dummyRestoredSeed = new QLineEdit(this);
    dummyRestoredSeed->setVisible(false);
    auto *restoreHeightEdit = new QLineEdit(this);
    restoreHeightEdit->setVisible(false);

    this->registerField("mnemonicRestoredSeed", dummyRestoredSeed);
    this->registerField("restoreHeight", restoreHeightEdit);

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::restoreOverrideCursor();
#endif

    connect(ui->seedBtnGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button){
        auto name = button->objectName();
        if(name == "radio25") {
            m_mode = 25;
            ui->label_errorString->hide();
            ui->seedEdit->setStyleSheet("");
            ui->seedEdit->setCompleter(m_completer25);
            ui->restoreFrame->show();
            ui->seedEdit->setPlaceholderText("Insert your mnemonic 25 word seed...");
        } else if (name  == "radio14") {
            m_mode = 14;
            ui->label_errorString->hide();
            ui->seedEdit->setStyleSheet("");
            ui->seedEdit->setCompleter(m_completer14);
            ui->restoreFrame->hide();
            ui->seedEdit->setPlaceholderText("Insert your mnemonic 14 word seed...");
        }

        ui->seedEdit->setText("");
    });

    if(m_ctx->networkType == NetworkType::Type::TESTNET) {
        ui->restoreHeightWidget->hideSlider();
    } else {
        // load restoreHeight lookup db
        ui->restoreHeightWidget->initRestoreHeights(m_ctx->restoreHeights[m_ctx->networkType]);
    }
}

int RestorePage::nextId() const {
    return WalletWizard::Page_CreateWallet;
}

void RestorePage::cleanupPage() const {}

bool RestorePage::validatePage() {
    ui->label_errorString->hide();
    auto errStyle = "QTextEdit{border: 1px solid red;}";
    int restoreHeight = ui->restoreHeightWidget->getHeight();
    auto seed = ui->seedEdit->toPlainText().replace("\n", "").replace("\r", "").trimmed();
    auto seedSplit = seed.split(" ");

    if(m_mode == 14) {
        if(seedSplit.length() != 14) {
            ui->label_errorString->show();
            ui->label_errorString->setText("The mnemonic seed should be 14 words.");
            ui->seedEdit->setStyleSheet(errStyle);
            return false;
        }

        for(const auto &word: seedSplit) {
            if(!m_words14.contains(word)) {
                ui->label_errorString->show();
                ui->label_errorString->setText(QString("Mnemonic seed contains an unknown word: %1").arg(word));
                ui->seedEdit->setStyleSheet(errStyle);
                return false;
            }
        }

        auto _seed = FeatherSeed::fromSeed(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName.toStdString(), m_ctx->seedLanguage, seed.toStdString());
        restoreHeight = _seed.restoreHeight;

        this->setField("restoreHeight", restoreHeight);
        this->setField("mnemonicRestoredSeed", seed);
        return true;
    } else if(m_mode == 25) {
        if(seedSplit.length() != 25) {
            ui->label_errorString->show();
            ui->label_errorString->setText("The mnemonic seed should be 25 words.");
            ui->seedEdit->setStyleSheet(errStyle);
            return false;
        }

        for(const auto &word: seedSplit) {
            if(!m_words25.contains(word)) {
                ui->label_errorString->show();
                ui->label_errorString->setText(QString("Mnemonic seed contains an unknown word: %1").arg(word));
                ui->seedEdit->setStyleSheet(errStyle);
                return false;
            }
        }

        auto _seed = FeatherSeed::fromSeed(m_ctx->restoreHeights[m_ctx->networkType], m_ctx->coinName.toStdString(), m_ctx->seedLanguage, seed.toStdString());
        _seed.setRestoreHeight(restoreHeight);
        this->setField("restoreHeight", restoreHeight);
        this->setField("mnemonicSeed", seed);
        this->setField("mnemonicRestoredSeed", seed);
        return true;
    }

    ui->seedEdit->setStyleSheet(errStyle);
    return false;
}
