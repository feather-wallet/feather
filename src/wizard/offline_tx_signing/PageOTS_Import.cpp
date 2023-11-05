// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Projecteated by user on 11/5/23.

#include "PageOTS_Import.h"
#include "ui_PageOTS_Import.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>

#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

PageOTS_Import::PageOTS_Import(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields, const QString &type, const QString &successButtonText)
        : QWizardPage(parent)
        , m_wallet(wallet)
        , m_wizardFields(wizardFields)
        , m_scanWidget(wizardFields->scanWidget)
        , m_type(type)
        , m_successButtonText(successButtonText)
        , ui(new Ui::PageOTS_Import)
{
    ui->setupUi(this);

    this->setTitle(QString("Import %1").arg(m_type));
    this->setCommitPage(true);
    this->setButtonText(QWizard::CommitButton, "Next");
    this->setButtonText(QWizard::FinishButton, "Next");

    ui->label_step->hide();
    ui->frame_status->hide();

    connect(ui->btn_import, &QPushButton::clicked, this, &PageOTS_Import::importFromFile);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });

    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
}

void PageOTS_Import::onScanFinished(bool success) {
    if (!success) {
        Utils::showError(this, "Failed to scan QR code", m_scanWidget->getURError());
        m_scanWidget->reset();
        return;
    }

    std::string data = m_scanWidget->getURData();
    importFromStr(data);
}

void PageOTS_Import::onSuccess() {
    m_success = true;
    emit completeChanged();
    
    this->wizard()->button(QWizard::CommitButton)->click();
    this->wizard()->button(QWizard::FinishButton)->click();
    
    ui->frame_status->show();
    ui->frame_status->setInfo(icons()->icon("confirmed.svg"), QString("%1 imported successfully").arg(m_type));
    this->setButtonText(QWizard::FinishButton, m_successButtonText);
}

void PageOTS_Import::initializePage() {
    m_scanWidget->reset();
    connect(m_scanWidget, &QrCodeScanWidget::finished, this, &PageOTS_Import::onScanFinished);
    ui->layout_scanner->addWidget(m_scanWidget);
    m_scanWidget->startCapture(true);
}

bool PageOTS_Import::isComplete() const {
    return m_success;
}

bool PageOTS_Import::validatePage() {
    m_scanWidget->disconnect();
    return true;
}