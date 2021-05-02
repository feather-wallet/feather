// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WalletWizard.h"
#include "PageWalletFile.h"
#include "ui_PageWalletFile.h"

#include "utils/utils.h"

#include <QFileDialog>
#include <QMessageBox>

PageWalletFile::PageWalletFile(AppContext *ctx, WizardFields *fields, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageWalletFile)
    , m_ctx(ctx)
    , m_fields(fields)
{
    ui->setupUi(this);
    this->setButtonText(QWizard::FinishButton, "Open wallet");

    QPixmap pixmap = QPixmap(":/assets/images/file.png");
    ui->lockIcon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    connect(ui->btnChange, &QPushButton::clicked, [=] {
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", m_ctx->defaultWalletDir, QFileDialog::ShowDirsOnly);
        if(walletDir.isEmpty()) return;
        m_ctx->defaultWalletDir = walletDir;
        ui->line_walletDir->setText(walletDir);
        config()->set(Config::walletDirectory, walletDir);
        emit defaultWalletDirChanged(walletDir);
    });

    connect(ui->line_walletName, &QLineEdit::textChanged, this, &PageWalletFile::validateWidgets);
    connect(ui->line_walletDir, &QLineEdit::textChanged, this, &PageWalletFile::validateWidgets);
}

void PageWalletFile::initializePage() {
    this->setTitle(m_fields->modeText);
    ui->line_walletDir->setText(m_ctx->defaultWalletDir);
    ui->line_walletName->setText(this->defaultWalletName());
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

    return true;
}

bool PageWalletFile::isComplete() const {
    return m_validated;
}

QString PageWalletFile::defaultWalletName() {
    int count = 1;
    QString walletName;
    do {
        walletName = QString("wallet_%1").arg(count);
        count++;
    } while (this->walletPathExists(walletName));

    return walletName;
}

bool PageWalletFile::walletPathExists(const QString &walletName) {
    QDir walletDir = QDir(ui->line_walletDir->text());

    return QFile::exists(walletDir.filePath(walletName)) ||
           QFile::exists(walletDir.filePath(QString("%1.keys").arg(walletName)));
}