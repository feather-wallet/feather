// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ExportOutputs.h"
#include "ui_PageOTS_Export.h"
#include "OfflineTxSigningWizard.h"

#include <QFileDialog>
#include <QCheckBox>

#include "utils/Utils.h"
#include "utils/config.h"

PageOTS_ExportOutputs::PageOTS_ExportOutputs(QWidget *parent, Wallet *wallet)
        : QWizardPage(parent)
        , ui(new Ui::PageOTS_Export)
        , m_wallet(wallet)
        , m_check_exportAll(new QCheckBox(this))
{
    ui->setupUi(this);
    this->setTitle("Export outputs");

    ui->label_step->hide();
    ui->label_instructions->setText("Scan this animated QR code with your offline wallet (Tools → Offline Transaction Signing).");

    m_check_exportAll->setText("Export all outputs");
    ui->layout_extra->addWidget(m_check_exportAll);
    connect(m_check_exportAll, &QCheckBox::toggled, this, &PageOTS_ExportOutputs::setupUR);
    
    connect(ui->btn_export, &QPushButton::clicked, this, &PageOTS_ExportOutputs::exportOutputs);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });
    
    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
}

void PageOTS_ExportOutputs::exportOutputs() {
    QString fn = QFileDialog::getSaveFileName(this, "Save outputs to file", QString("%1/%2_%3").arg(QDir::homePath(), m_wallet->walletName(), QString::number(QDateTime::currentSecsSinceEpoch())), "Outputs (*_outputs)");
    if (fn.isEmpty()) {
        return;
    }
    if (!fn.endsWith("_outputs")) {
        fn += "_outputs";
    }

    bool r = m_wallet->exportOutputs(fn, m_check_exportAll->isChecked());
    if (!r) {
        Utils::showError(this, "Failed to export outputs", m_wallet->errorString());
        return;
    } 
    
    Utils::showInfo(this, "Successfully exported outputs.");
}

void PageOTS_ExportOutputs::setupUR(bool all) {
    std::string output_export;
    m_wallet->exportOutputsToStr(output_export, all);
    ui->widget_UR->setData("xmr-output", output_export);
}

void PageOTS_ExportOutputs::initializePage() {
    this->setupUR(false);
}

int PageOTS_ExportOutputs::nextId() const {
    return OfflineTxSigningWizard::Page_ImportKeyImages;
}
