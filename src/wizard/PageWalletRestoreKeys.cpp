// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageWalletRestoreKeys.h"
#include "ui_PageWalletRestoreKeys.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPushButton>

#include "WalletWizard.h"
#include "constants.h"
#include "dialog/URDialog.h"
#include "libwalletqt/WalletManager.h"
#include "scanner/QrCodeScanDialog.h"

PageWalletRestoreKeys::PageWalletRestoreKeys(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletRestoreKeys)
    , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Restore wallet from keys");
    ui->label_errorString->hide();

#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QGuiApplication::restoreOverrideCursor();
#endif

    if (constants::networkType == NetworkType::Type::MAINNET) {
        ui->line_address->setPlaceholderText("4...");
    } else if (constants::networkType == NetworkType::Type::STAGENET) {
        ui->line_address->setPlaceholderText("5...");
    }

    QRegularExpression keyRe(R"([0-9a-fA-F]{64})");
    QValidator *keyValidator = new QRegularExpressionValidator(keyRe, this);

    ui->line_viewkey->setValidator(keyValidator);
    ui->line_spendkey->setValidator(keyValidator);

    connect(ui->btnOptions, &QPushButton::clicked, this, &PageWalletRestoreKeys::onOptionsClicked);
    connect(ui->combo_walletType, &QComboBox::currentTextChanged, this, &PageWalletRestoreKeys::showInputLines);
    connect(ui->btn_scanUR, &QPushButton::clicked, [this] {
        QrCodeScanDialog dialog{this, false};
        dialog.exec();

        QString json = dialog.decodedString();
        if (json.isEmpty()) {
            return;
        }

        QJsonParseError error;
        auto doc = QJsonDocument::fromJson(json.toUtf8(), &error);
        if (error.error != QJsonParseError::NoError) {
            Utils::showError(this, "Unable to load view-only details", QString("Can't parse JSON: %1").arg(error.errorString()));
            return;
        }

        ui->line_address->setText(doc["primaryAddress"].toString());
        ui->line_address->setCursorPosition(0);
        ui->line_viewkey->setText(doc["privateViewKey"].toString());
        ui->line_viewkey->setCursorPosition(0);
        m_fields->restoreHeight = doc["restoreHeight"].toInt();
        m_fields->walletName = doc["walletName"].toString() + "_view_only";
    });
}

void PageWalletRestoreKeys::initializePage() {
    this->showInputLines();
}

int PageWalletRestoreKeys::nextId() const {
    return WalletWizard::Page_SetRestoreHeight;
}

void PageWalletRestoreKeys::showInputLines() {
    ui->label_errorString->hide();

    if (ui->combo_walletType->currentIndex() == walletType::ViewOnly) {
        ui->frame_address->show();
        ui->frame_viewKey->show();
        ui->frame_spendKey->hide();
    }
    else if (ui->combo_walletType->currentIndex() == walletType::Spendable) {
        ui->frame_address->hide();
        ui->frame_viewKey->hide();
        ui->frame_spendKey->show();
    }
    else {
        ui->frame_address->show();
        ui->frame_viewKey->show();
        ui->frame_spendKey->show();
    }

    ui->line_address->setText("");
    ui->line_viewkey->setText("");
    ui->line_spendkey->setText("");
}

bool PageWalletRestoreKeys::validatePage() {
    auto errStyle = "QLineEdit{border: 1px solid red;}";

    ui->line_address->setStyleSheet("");
    ui->line_viewkey->setStyleSheet("");
    ui->label_errorString->hide();

    QString address = ui->line_address->text().trimmed();
    QString viewkey = ui->line_viewkey->text().trimmed();
    QString spendkey = ui->line_spendkey->text().trimmed();

    if (walletType() == walletType::ViewOnly || walletType() == walletType::Spendable_Nondeterministic) {
        if (!WalletManager::addressValid(address, constants::networkType)){
            ui->label_errorString->show();
            ui->label_errorString->setText("Error: Invalid address.");
            ui->line_address->setStyleSheet(errStyle);
            return false;
        }

        if (!WalletManager::keyValid(viewkey, address, true, constants::networkType)) {
            ui->label_errorString->show();
            ui->label_errorString->setText("Error: Invalid key.");
            ui->line_viewkey->setStyleSheet(errStyle);
            return false;
        }
    }

    if (walletType() == walletType::Spendable || walletType() == walletType::Spendable_Nondeterministic) {
        bool spendKeyValid = (ui->line_spendkey->hasAcceptableInput() && walletType() == walletType::Spendable)
                || (WalletManager::keyValid(spendkey, address, false, constants::networkType) && walletType() == walletType::Spendable_Nondeterministic);

        if (!spendKeyValid) {
            ui->label_errorString->show();
            ui->label_errorString->setText("Error: Invalid key.");
            ui->line_spendkey->setStyleSheet(errStyle);
            return false;
        }
    }

    m_fields->address = address;
    m_fields->secretViewKey = viewkey;
    m_fields->secretSpendKey = spendkey;
    return true;
}

void PageWalletRestoreKeys::onOptionsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Options");

    QVBoxLayout layout;
    QCheckBox check_subaddressLookahead("Set subaddress lookahead");
    check_subaddressLookahead.setChecked(m_fields->showSetSubaddressLookaheadPage);

    layout.addWidget(&check_subaddressLookahead);
    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();

    m_fields->showSetSubaddressLookaheadPage = check_subaddressLookahead.isChecked();
}

int PageWalletRestoreKeys::walletType() {
    return ui->combo_walletType->currentIndex();
}