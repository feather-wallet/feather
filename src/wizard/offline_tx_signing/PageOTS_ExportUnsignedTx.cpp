// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "PageOTS_ExportUnsignedTx.h"
#include "ui_PageOTS_Export.h"
#include "OfflineTxSigningWizard.h"

#include "utils/Utils.h"
#include "utils/config.h"

PageOTS_ExportUnsignedTx::PageOTS_ExportUnsignedTx(QWidget *parent, Wallet *wallet, PendingTransaction *tx)
        : QWizardPage(parent)
        , ui(new Ui::PageOTS_Export)
        , m_wallet(wallet)
        , m_tx(tx)
{
    ui->setupUi(this);
    this->setTitle("3. Export unsigned transaction");

    ui->label_step->hide();
    ui->label_instructions->setText("Scan this animated QR code with the offline wallet.");

    connect(ui->btn_export, &QPushButton::clicked, this, &PageOTS_ExportUnsignedTx::exportUnsignedTx);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });
}

void PageOTS_ExportUnsignedTx::initializePage() {
    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
    ui->widget_UR->setData("xmr-txunsigned", m_tx->unsignedTxToBin());
}

void PageOTS_ExportUnsignedTx::exportUnsignedTx() {
    QString defaultName = QString("%1_unsigned_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = Utils::getSaveFileName(this, "Save transaction to file", defaultName, "Transaction (*unsigned_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }
    
    bool r = m_tx->saveToFile(fn);
    if (!r) {
        Utils::showError(this, "Failed to export unsigned transaction", m_wallet->errorString());
        return;
    } 
    
    QFileInfo fileInfo(fn);
    Utils::openDir(this, "Successfully exported unsigned transaction", fileInfo.absolutePath());
}

int PageOTS_ExportUnsignedTx::nextId() const {
    return OfflineTxSigningWizard::Page_ImportSignedTx;
}
