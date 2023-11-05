// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ExportSignedTx.h"
#include "ui_PageOTS_Export.h"

#include <QFileDialog>

#include "OfflineTxSigningWizard.h"
#include "utils/config.h"
#include "utils/Utils.h"

PageOTS_ExportSignedTx::PageOTS_ExportSignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : QWizardPage(parent)
        , ui(new Ui::PageOTS_Export)
        , m_wallet(wallet)
        , m_wizardFields(wizardFields)
{
    ui->setupUi(this);
    
    this->setTitle("Export signed transaction");
    
    ui->label_step->hide();
    ui->label_instructions->setText("Scan this animated QR code with your view-only wallet.");

    connect(ui->btn_export, &QPushButton::clicked, this, &PageOTS_ExportSignedTx::exportSignedTx);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });

    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
}

void PageOTS_ExportSignedTx::exportSignedTx() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save signed transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }

    bool r = m_wizardFields->utx->sign(fn);

    if (!r) {
        Utils::showError(this, "Failed to save transaction to file");
        return;
    }

    Utils::showInfo(this, "Transaction saved successfully");
}

void PageOTS_ExportSignedTx::initializePage() {
    if (m_wizardFields->utx) {
        m_wizardFields->utx->signToStr(m_wizardFields->signedTx);
    }
    
    // TODO: check that signedTx is not empty
    
    ui->widget_UR->setData("xmr-txsigned", m_wizardFields->signedTx);
}

int PageOTS_ExportSignedTx::nextId() const {
    return -1;
}
