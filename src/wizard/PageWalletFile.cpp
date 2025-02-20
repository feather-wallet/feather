// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "WalletWizard.h"
#include "PageWalletFile.h"
#include "ui_PageWalletFile.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

PageWalletFile::PageWalletFile(WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletFile)
    , m_fields(fields)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    ui->frame_wallet->setInfo(icons()->icon("file"), "Choose a name and directory for your wallet files.");

    QRegularExpression nameRe(R"([^\/\\:*?"<>|]+)");
    QValidator *nameValidator = new QRegularExpressionValidator(nameRe, this);
    ui->line_walletName->setValidator(nameValidator);

    connect(ui->btnChange, &QPushButton::clicked, [=] {
        QString currentWalletDir = conf()->get(Config::walletDirectory).toString();
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", currentWalletDir, QFileDialog::ShowDirsOnly);
        if (walletDir.isEmpty()) {
            return;
        }
        ui->line_walletDir->setText(walletDir);
    });

    connect(ui->line_walletName, &QLineEdit::textChanged, this, &PageWalletFile::validateWidgets);
    connect(ui->line_walletDir, &QLineEdit::textChanged, this, [this](){
        ui->check_defaultWalletDirectory->setVisible(true);
        this->validateWidgets();
    });
}

void PageWalletFile::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->line_walletDir->setText(conf()->get(Config::walletDirectory).toString());
    ui->line_walletName->setText(this->defaultWalletName());
    ui->check_defaultWalletDirectory->setVisible(false);
    ui->check_defaultWalletDirectory->setChecked(false);

    if (!m_fields->walletName.isEmpty()) {
        ui->line_walletName->setText(m_fields->walletName);
    }
}

bool PageWalletFile::validateWidgets(){
    QString walletName = ui->line_walletName->text();
    QString walletDir = ui->line_walletDir->text();

    m_validated = true;
    ui->line_walletName->setStyleSheet("");
    ui->line_walletDir->setStyleSheet("");
    QString errStyle = "QLineEdit{border: 1px solid red;}";

    if (walletDir.isEmpty()) {
        ui->line_walletDir->setStyleSheet(errStyle);
        m_validated = false;
    }

    if (!Utils::dirExists(walletDir)) {
        ui->line_walletDir->setStyleSheet(errStyle);
        m_validated = false;
    }

    if (walletName.isEmpty()) {
        ui->line_walletName->setStyleSheet(errStyle);
        m_validated = false;
    }

    if (this->walletPathExists(walletName)) {
        ui->line_walletName->setStyleSheet(errStyle);
        m_validated = false;
    }

    this->completeChanged();
    return m_validated;
}

int PageWalletFile::nextId() const {
    return WalletWizard::Page_SetPasswordPage;
}

bool PageWalletFile::validatePage() {
    if (!this->validateWidgets()) {
        return false;
    }

    m_fields->walletName = ui->line_walletName->text();
    m_fields->walletDir = ui->line_walletDir->text();

    QString walletDir = ui->line_walletDir->text();
    bool dirChanged = conf()->get(Config::walletDirectory).toString() != walletDir;
    if (dirChanged && ui->check_defaultWalletDirectory->isChecked()) {
        conf()->set(Config::walletDirectory, walletDir);
    }

    return true;
}

bool PageWalletFile::isComplete() const {
    return m_validated;
}

QString PageWalletFile::defaultWalletName() {
    int count = 1;
    QString walletName;
    do {
        QString walletStr = QString("wallet_%1");
        if (m_fields->mode == WizardMode::CreateWalletFromDevice) {
            switch (m_fields->deviceType) {
                case DeviceType::LEDGER:
                    walletStr = QString("ledger_%1");
                    break;
                case DeviceType::TREZOR:
                    walletStr = QString("trezor_%1");
            }
        }
        walletName = walletStr.arg(count);
        count++;
    } while (this->walletPathExists(walletName));

    return walletName;
}

bool PageWalletFile::walletPathExists(const QString &walletName) {
    QDir walletDir = QDir(ui->line_walletDir->text());

    return QFile::exists(walletDir.filePath(walletName)) ||
           QFile::exists(walletDir.filePath(QString("%1.keys").arg(walletName)));
}