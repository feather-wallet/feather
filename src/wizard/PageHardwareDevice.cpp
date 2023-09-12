// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageHardwareDevice.h"
#include "ui_PageHardwareDevice.h"
#include "WalletWizard.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>

PageHardwareDevice::PageHardwareDevice(WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageHardwareDevice)
        , m_fields(fields)
{
    ui->setupUi(this);

    ui->combo_deviceType->addItem("Ledger Nano S (PLUS) / X", DeviceType::LEDGER);
    ui->combo_deviceType->addItem("Trezor Model T", DeviceType::TREZOR);

    connect(ui->btnOptions, &QPushButton::clicked, this, &PageHardwareDevice::onOptionsClicked);
}

void PageHardwareDevice::initializePage() {
    ui->radioNewWallet->setChecked(true);
}

int PageHardwareDevice::nextId() const {
    if (m_fields->showSetRestoreHeightPage) {
        return WalletWizard::Page_SetRestoreHeight;
    }

    return WalletWizard::Page_WalletFile;
}

bool PageHardwareDevice::validatePage() {
    m_fields->deviceType = static_cast<DeviceType>(ui->combo_deviceType->currentData().toInt());
    m_fields->showSetRestoreHeightPage = ui->radioRestoreWallet->isChecked();
    return true;
}

bool PageHardwareDevice::isComplete() const {
    return true;
}

void PageHardwareDevice::onOptionsClicked() {
    QDialog dialog(this);
    dialog.setWindowTitle("Options");

    QVBoxLayout layout;
    QCheckBox check_subaddressLookahead("Set subaddress lookahead");
    check_subaddressLookahead.setChecked(m_fields->showSetSubaddressLookaheadPage);

    layout.addWidget(&check_subaddressLookahead);
    QDialogButtonBox buttons(QDialogButtonBox::Ok);
    layout.addWidget(&buttons);
    dialog.setLayout(&layout);
    connect(&buttons, &QDialogButtonBox::accepted, [&dialog]{
        dialog.close();
    });
    dialog.exec();

    m_fields->showSetSubaddressLookaheadPage = check_subaddressLookahead.isChecked();
}