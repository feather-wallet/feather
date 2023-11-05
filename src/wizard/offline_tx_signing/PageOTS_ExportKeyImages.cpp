// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ExportKeyImages.h"
#include "ui_PageOTS_Export.h"
#include "OfflineTxSigningWizard.h"

#include <QCheckBox>

#include "utils/config.h"
#include "utils/Utils.h"

PageOTS_ExportKeyImages::PageOTS_ExportKeyImages(QWidget *parent, Wallet *wallet)
        : QWizardPage(parent)
        , ui(new Ui::PageOTS_Export)
        , m_wallet(wallet)
{
    ui->setupUi(this);
    this->setTitle("Export key images");
    
    ui->label_step->hide();
    ui->label_instructions->setText("Scan this animated QR code with the view-only wallet.");

    auto check_exportAll = new QCheckBox(this);
    check_exportAll->setText("Export all key images");
    ui->layout_extra->addWidget(check_exportAll);
    connect(check_exportAll, &QCheckBox::toggled, this, &PageOTS_ExportKeyImages::setupUR);
    
    connect(ui->btn_export, &QPushButton::clicked, this, &PageOTS_ExportKeyImages::exportKeyImages);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });

    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
}

void PageOTS_ExportKeyImages::exportKeyImages() {
    QString fn = QFileDialog::getSaveFileName(this, "Save key images to file", QString("%1/%2_%3").arg(QDir::homePath(), m_wallet->walletName(), QString::number(QDateTime::currentSecsSinceEpoch())), "Key Images (*_keyImages)");
    if (fn.isEmpty()) {
        return;
    }
    if (!fn.endsWith("_keyImages")) {
        fn += "_keyImages";
    }
    
    bool r = m_wallet->exportKeyImages(fn, true);
    if (!r) {
        Utils::showError(this, "Failed to export key images", m_wallet->errorString());
        return;
    }
    
    Utils::showInfo(this, "Successfully exported key images");
}

void PageOTS_ExportKeyImages::setupUR(bool all) {
    std::string ki_export;
    m_wallet->exportKeyImagesToStr(ki_export, all);
    ui->widget_UR->setData("xmr-keyimage", ki_export);
}

void PageOTS_ExportKeyImages::initializePage() {
    this->setupUR(false);
}

int PageOTS_ExportKeyImages::nextId() const {
    return OfflineTxSigningWizard::Page_ImportUnsignedTx;
}
