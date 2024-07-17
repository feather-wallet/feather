// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PageMultisigSetupDebug.h"
#include "ui_PageMultisigSetupDebug.h"

#include <QFileDialog>

#include "utils/Icons.h"
#include "utils/Utils.h"

#include "ringct/rctOps.h"
#include "string_tools.h"
#include "libwalletqt/MultisigMessageStore.h"

#include "MMSWidget.h"

PageMultisigSetupDebug::PageMultisigSetupDebug(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageMultisigSetupDebug)
        , m_fields(fields)
{
    ui->setupUi(this);
    this->setTitle("Creating multisig wallet");
    this->setStatus("Preparing multisig");
//    this->setCommitPage(true);
}

void PageMultisigSetupDebug::initializePage() {
//    m_mmsWidget = new MMSWidget(m_fields->wallet, this);
//    ui->verticalLayout->addWidget(m_mmsWidget);
//    m_mmsWidget->setModel(m_fields->wallet->mmsModel(), m_fields->wallet->mmsStore());
    m_fields->wallet->setMMSRefreshEnabled(true);
    connect(m_fields->wallet->mmsStore(), &MultisigMessageStore::multisigWalletCreated, this, &PageMultisigSetupDebug::onMultisigWalletCreated);
    connect(m_fields->wallet->mmsStore(), &MultisigMessageStore::statusChanged, [this](const QString &status){
       this->setStatus(status);
    });
    connect(ui->btn_copyAddress, &QPushButton::clicked, [this]{
       Utils::copyToClipboard(m_fields->wallet->address(0, 0));
    });
    ui->frame_walletCreated->hide();
}

int PageMultisigSetupDebug::nextId() const {
    return WalletWizard::Page_MultisigSeed;
}

bool PageMultisigSetupDebug::validatePage() {
    return true;
}

bool PageMultisigSetupDebug::isComplete() const {
    return true;
}

void PageMultisigSetupDebug::onMultisigWalletCreated(const QString &address) {
    this->setStatus(QString("Multisig wallet has been successfully created."));
    ui->frame_walletCreated->show();
    ui->label_address->setText(Utils::chunkAddress(address));
}

void PageMultisigSetupDebug::setStatus(const QString &status) {
    ui->label_status->setText(QString("Status: %1").arg(status));
}