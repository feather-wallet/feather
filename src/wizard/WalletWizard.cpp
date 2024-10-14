// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "WalletWizard.h"

#include <QPushButton>

#include "PageMenu.h"
#include "PageOpenWallet.h"
#include "PagePlugins.h"
#include "PageWalletFile.h"
#include "PageNetwork.h"
#include "PageWalletSeed.h"
#include "PageWalletRestoreSeed.h"
#include "PageWalletRestoreKeys.h"
#include "PageSetPassword.h"
#include "PageSetRestoreHeight.h"
#include "PageSetSeedPassphrase.h"
#include "PageSetSubaddressLookahead.h"
#include "PageHardwareDevice.h"
#include "PageNetworkProxy.h"
#include "PageNetworkWebsocket.h"
#include "constants.h"
#include "WindowManager.h"
#include "utils/AppData.h"

WalletWizard::WalletWizard(QWidget *parent)
    : QWizard(parent)
{
    this->setWindowTitle("Welcome to Feather Wallet");
    this->setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));

    m_walletKeysFilesModel = new WalletKeysFilesModel(this);
    m_walletKeysFilesModel->refresh();

    auto networkPage = new PageNetwork(this);
    auto networkProxyPage = new PageNetworkProxy(this);
    auto networkWebsocketPage = new PageNetworkWebsocket(this);
    auto menuPage = new PageMenu(&m_wizardFields, m_walletKeysFilesModel, this);
    auto openWalletPage = new PageOpenWallet(m_walletKeysFilesModel, this);
    auto createWallet = new PageWalletFile(&m_wizardFields , this);
    auto createWalletSeed = new PageWalletSeed(&m_wizardFields, this);
    auto walletSetPasswordPage = new PageSetPassword(&m_wizardFields, this);
    auto walletSetSeedPassphrasePage = new PageSetSeedPassphrase(&m_wizardFields, this);
    auto walletSetSubaddressLookaheadPage = new PageSetSubaddressLookahead(&m_wizardFields, this);
    setPage(Page_Menu, menuPage);
    setPage(Page_WalletFile, createWallet);
    setPage(Page_OpenWallet, openWalletPage);
    setPage(Page_CreateWalletSeed, createWalletSeed);
    setPage(Page_SetPasswordPage, walletSetPasswordPage);
    setPage(Page_Network, networkPage);
    setPage(Page_NetworkProxy, networkProxyPage);
    setPage(Page_NetworkWebsocket, networkWebsocketPage);
    setPage(Page_WalletRestoreSeed, new PageWalletRestoreSeed(&m_wizardFields, this));
    setPage(Page_WalletRestoreKeys, new PageWalletRestoreKeys(&m_wizardFields, this));
    setPage(Page_SetRestoreHeight, new PageSetRestoreHeight(&m_wizardFields, this));
    setPage(Page_HardwareDevice, new PageHardwareDevice(&m_wizardFields, this));
    setPage(Page_SetSeedPassphrase, walletSetSeedPassphrasePage);
    setPage(Page_SetSubaddressLookahead, walletSetSubaddressLookaheadPage);
    setPage(Page_Plugins, new PagePlugins(this));

    setStartId(Page_Menu);

    setButtonText(QWizard::CancelButton, "Close");
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/assets/images/banners/3.png"));
    setWizardStyle(WizardStyle::ModernStyle);
    setOption(QWizard::NoBackButtonOnStartPage);
    setOption(QWizard::HaveHelpButton, true);
    setOption(QWizard::HaveCustomButton1, true);

    QList<QWizard::WizardButton> layout;
    layout << QWizard::HelpButton;
    layout << QWizard::CustomButton1;
    layout << QWizard::Stretch;
    layout << QWizard::BackButton;
    layout << QWizard::NextButton;
    layout << QWizard::FinishButton;
    layout << QWizard::CommitButton;
    this->setButtonLayout(layout);

    auto *settingsButton = new QPushButton("Settings", this);
    this->setButton(QWizard::CustomButton1, settingsButton);

    settingsButton->setVisible(false);
    connect(this, &QWizard::currentIdChanged, [this, settingsButton](int currentId){
        settingsButton->setVisible(currentId == Page_Menu);

        auto helpButton = this->button(QWizard::HelpButton);
        helpButton->setVisible(!this->helpPage().isEmpty());
    });
    connect(settingsButton, &QPushButton::clicked, this, &WalletWizard::showSettings);

    connect(networkWebsocketPage, &PageNetworkWebsocket::initialNetworkConfigured, [this](){
        emit initialNetworkConfigured();
    });

    connect(walletSetPasswordPage, &PageSetPassword::createWallet, this, &WalletWizard::onCreateWallet);

    connect(openWalletPage, &PageOpenWallet::openWallet, [=](const QString &path){
        emit openWallet(path, "");
    });

    connect(this, &QWizard::helpRequested, this, &WalletWizard::showHelp);
}

void WalletWizard::resetFields() {
    m_wizardFields = {};
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
            case DeviceType::LEDGER:
                deviceName = "Ledger";
                break;
            case DeviceType::TREZOR:
                deviceName = "Trezor";
        }

        emit createWalletFromDevice(walletPath, m_wizardFields.password, deviceName, restoreHeight, m_wizardFields.subaddressLookahead);
        return;
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromKeys) {
        emit createWalletFromKeys(walletPath,
                                  m_wizardFields.password,
                                  m_wizardFields.address,
                                  m_wizardFields.secretViewKey,
                                  m_wizardFields.secretSpendKey,
                                  m_wizardFields.restoreHeight,
                                  m_wizardFields.subaddressLookahead);
        return;
    }

    // If we're connected to the websocket, use the reported height for new wallets to skip initial synchronization.
    if (m_wizardFields.mode == WizardMode::CreateWallet && currentBlockHeight > 0) {
        qInfo() << "New wallet, setting restore height to latest blockheight: " << currentBlockHeight;
        m_wizardFields.seed.restoreHeight = currentBlockHeight;
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromSeed && (m_wizardFields.seedType == Seed::Type::MONERO || m_wizardFields.showSetRestoreHeightPage)) {
        m_wizardFields.seed.setRestoreHeight(m_wizardFields.restoreHeight);
    }

    bool newWallet = m_wizardFields.mode == WizardMode::CreateWallet;

    emit createWallet(m_wizardFields.seed, walletPath, m_wizardFields.password, m_wizardFields.seedLanguage, m_wizardFields.seedOffsetPassphrase, m_wizardFields.subaddressLookahead, newWallet);
}

QString WalletWizard::helpPage() {
    QString doc;
    switch (this->currentId()) {
        case Page_Menu: {
            doc = "about";
            break;
        }
        case Page_CreateWalletSeed: {
            doc = "seed_scheme";
            break;
        }
        case Page_WalletFile: {
            doc = "wallet_files";
            break;
        }
        case Page_HardwareDevice: {
            doc = "create_wallet_hardware_device";
            break;
        }
        case Page_SetRestoreHeight: {
            doc = "restore_height";
            break;
        }
    }
    return doc;
}

void WalletWizard::showHelp() {
    QString doc = this->helpPage();

    if (!doc.isEmpty()) {
        windowManager()->showDocs(this, doc);
    }
}