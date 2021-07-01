// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "utils/utils.h"

#include "WalletWizard.h"
#include "PageMenu.h"
#include "PageOpenWallet.h"
#include "PageWalletFile.h"
#include "PageNetwork.h"
#include "PageWalletSeed.h"
#include "PageWalletRestoreSeed.h"
#include "PageWalletRestoreKeys.h"
#include "PageSetPassword.h"
#include "PageSetRestoreHeight.h"
#include "PageHardwareDevice.h"
#include "PageNetworkTor.h"
#include "constants.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QScreen>

WalletWizard::WalletWizard(QWidget *parent)
    : QWizard(parent)
{
    this->setWindowTitle("Welcome to Feather Wallet");
    this->setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));

    m_walletKeysFilesModel = new WalletKeysFilesModel(this);
    m_walletKeysFilesModel->refresh();

    auto networkPage = new PageNetwork(this);
    auto networkTorPage = new PageNetworkTor(this);
    auto menuPage = new PageMenu(&m_wizardFields, m_walletKeysFilesModel, this);
    auto openWalletPage = new PageOpenWallet(m_walletKeysFilesModel, this);
    auto createWallet = new PageWalletFile(&m_wizardFields , this);
    auto createWalletSeed = new PageWalletSeed(&m_wizardFields, this);
    auto walletSetPasswordPage = new PageSetPassword(&m_wizardFields, this);
    setPage(Page_Menu, menuPage);
    setPage(Page_WalletFile, createWallet);
    setPage(Page_OpenWallet, openWalletPage);
    setPage(Page_CreateWalletSeed, createWalletSeed);
    setPage(Page_SetPasswordPage, walletSetPasswordPage);
    setPage(Page_Network, networkPage);
    setPage(Page_NetworkTor, networkTorPage);
    setPage(Page_WalletRestoreSeed, new PageWalletRestoreSeed(&m_wizardFields, this));
    setPage(Page_WalletRestoreKeys, new PageWalletRestoreKeys(&m_wizardFields, this));
    setPage(Page_SetRestoreHeight, new PageSetRestoreHeight(&m_wizardFields, this));
    setPage(Page_HardwareDevice, new PageHardwareDevice(&m_wizardFields, this));

    setStartId(Page_Menu);

    setButtonText(QWizard::CancelButton, "Close");
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/assets/images/banners/3.png"));
    setWizardStyle(WizardStyle::ModernStyle);
    setOption(QWizard::NoBackButtonOnStartPage);

    connect(networkTorPage, &PageNetworkTor::initialNetworkConfigured, [this](){
        emit initialNetworkConfigured();
    });

    connect(menuPage, &PageMenu::enableDarkMode, [this](bool enable){
        if (enable)
            emit skinChanged("QDarkStyle");
        else
            emit skinChanged("Native");
    });

    connect(walletSetPasswordPage, &PageSetPassword::createWallet, this, &WalletWizard::onCreateWallet);


    connect(createWallet, &PageWalletFile::defaultWalletDirChanged, [this](const QString &walletDir){
        emit defaultWalletDirChanged(walletDir);
    });

    connect(openWalletPage, &PageOpenWallet::openWallet, [=](const QString &path){
        emit openWallet(path, "");
    });
}

void WalletWizard::onCreateWallet() {
    auto walletPath = QString("%1/%2").arg(m_wizardFields.walletDir, m_wizardFields.walletName);

    int currentBlockHeight = 0;
    if (appData()->heights.contains(constants::networkType)) {
        currentBlockHeight = appData()->heights[constants::networkType];
    }

    if (m_wizardFields.mode == WizardMode::CreateWalletFromDevice) {
        int restoreHeight = currentBlockHeight;
        if (m_wizardFields.restoreHeight > 0) {
            restoreHeight = m_wizardFields.restoreHeight;
        }

        QString deviceName;
        switch (m_wizardFields.deviceType) {
            case DeviceType::LEDGER_NANO_S:
            case DeviceType::LEDGER_NANO_X:
                deviceName = "Ledger";
                break;
            case DeviceType::TREZOR_MODEL_T:
                deviceName = "Trezor";
        }

        emit createWalletFromDevice(walletPath, m_wizardFields.password, deviceName, restoreHeight);
        return;
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromKeys) {
        emit createWalletFromKeys(walletPath,
                                  m_wizardFields.password,
                                  m_wizardFields.address,
                                  m_wizardFields.secretViewKey,
                                  m_wizardFields.secretSpendKey,
                                  m_wizardFields.restoreHeight);
        return;
    }

    auto seed = FeatherSeed(constants::networkType, QString::fromStdString(constants::coinName), constants::seedLanguage, m_wizardFields.seed.split(" "));

    if (m_wizardFields.mode == WizardMode::CreateWallet && currentBlockHeight > 0) {
        qInfo() << "New wallet, setting restore height to latest blockheight: " << currentBlockHeight;
        seed.setRestoreHeight(currentBlockHeight);
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromSeed && m_wizardFields.seedType == SeedType::MONERO)
        seed.setRestoreHeight(m_wizardFields.restoreHeight);

    emit createWallet(seed, walletPath, m_wizardFields.password, m_wizardFields.seedOffsetPassphrase);
}
