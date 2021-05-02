// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageHardwareDevice.h"
#include "ui_PageHardwareDevice.h"
#include "WalletWizard.h"

#include <QMessageBox>

PageHardwareDevice::PageHardwareDevice(AppContext *ctx, WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageHardwareDevice)
        , m_ctx(ctx)
        , m_fields(fields)
{
    ui->setupUi(this);
}

void PageHardwareDevice::initializePage() {
    ui->radioNewWallet->setChecked(true);
}

int PageHardwareDevice::nextId() const {
    if (ui->radioNewWallet->isChecked())
        return WalletWizard::Page_WalletFile;
    if (ui->radioRestoreWallet->isChecked())
        return WalletWizard::Page_SetRestoreHeight;
    return 0;
}

bool PageHardwareDevice::validatePage() {
    return true;
}

bool PageHardwareDevice::isComplete() const {
    return true;
}