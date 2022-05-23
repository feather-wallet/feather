// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "PageHardwareDevice.h"
#include "ui_PageHardwareDevice.h"
#include "WalletWizard.h"

#include <QMessageBox>

PageHardwareDevice::PageHardwareDevice(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageHardwareDevice)
        , m_fields(fields)
{
    ui->setupUi(this);

    ui->combo_deviceType->addItem("Ledger Nano S (PLUS) / X", DeviceType::LEDGER);
    ui->combo_deviceType->addItem("Trezor Model T", DeviceType::TREZOR);
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
    m_fields->deviceType = static_cast<DeviceType>(ui->combo_deviceType->currentData().toInt());
    return true;
}

bool PageHardwareDevice::isComplete() const {
    return true;
}