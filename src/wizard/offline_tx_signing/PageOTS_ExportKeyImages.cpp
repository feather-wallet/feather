// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_ExportKeyImages.h"
#include "ui_PageOTS_Export.h"
#include "OfflineTxSigningWizard.h"

#include <QCheckBox>

#include "utils/config.h"
#include "utils/Utils.h"

PageOTS_ExportKeyImages::PageOTS_ExportKeyImages(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields)
        : QWizardPage(parent)
        , ui(new Ui::PageOTS_Export)
        , m_wallet(wallet)
        , m_wizardFields(wizardFields)
{
    ui->setupUi(this);
    this->setTitle("2. Export key images");
    
    ui->label_step->hide();
    ui->label_instructions->setText("Scan this animated QR code with the view-only wallet.");

    connect(ui->btn_export, &QPushButton::clicked, this, &PageOTS_ExportKeyImages::exportKeyImages);
    connect(ui->combo_method, &QComboBox::currentIndexChanged, [this](int index){
        conf()->set(Config::offlineTxSigningMethod, index);
        ui->stackedWidget->setCurrentIndex(index);
    });
}

void PageOTS_ExportKeyImages::exportKeyImages() {
    QString defaultName = QString("%1_%2").arg(m_wallet->walletName(), QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = Utils::getSaveFileName(this, "Save key images to file", defaultName, "Key Images (*_keyImages)");
    if (fn.isEmpty()) {
        return;
    }
    if (!fn.endsWith("_keyImages")) {
        fn += "_keyImages";
    }

    QFile file{fn};
    if (!file.open(QIODevice::WriteOnly)) {
      Utils::showError(this, "Failed to export key images", QString("Could not open file %1 for writing").arg(fn));
      return;
    }

    file.write(m_wizardFields->keyImages.data(), m_wizardFields->keyImages.size());
    file.close();

    QFileInfo fileInfo(fn);
    Utils::openDir(this, "Successfully exported key images", fileInfo.absolutePath());
}

void PageOTS_ExportKeyImages::setupUR(bool all) {
    // TODO: check if empty
    std::string ki_export;
    m_wallet->exportKeyImagesToStr(ki_export, all);
    ui->widget_UR->setData("xmr-keyimage", m_wizardFields->keyImages);
}

void PageOTS_ExportKeyImages::initializePage() {
    ui->combo_method->setCurrentIndex(conf()->get(Config::offlineTxSigningMethod).toInt());
    this->setupUR(false);
}

int PageOTS_ExportKeyImages::nextId() const {
    return OfflineTxSigningWizard::Page_ImportUnsignedTx;
}
