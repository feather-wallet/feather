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
#include "globals.h"

#include <QLineEdit>
#include <QVBoxLayout>
#include <QScreen>

WalletWizard::WalletWizard(AppContext *ctx, WalletWizard::Page startPage, QWidget *parent)
        : QWizard(parent)
        , m_ctx(ctx)
{
    this->setWindowTitle("Welcome to Feather Wallet");
    this->setWindowIcon(QIcon(":/assets/images/appicons/64x64.png"));

    m_walletKeysFilesModel = new WalletKeysFilesModel(m_ctx, this);
    m_walletKeysFilesModel->refresh();

    auto networkPage = new PageNetwork(m_ctx, this);
    auto networkTorPage = new PageNetworkTor(m_ctx, this);
    auto menuPage = new PageMenu(m_ctx, &m_wizardFields, m_walletKeysFilesModel, this);
    auto openWalletPage = new PageOpenWallet(m_ctx, m_walletKeysFilesModel, this);
    auto createWallet = new PageWalletFile(m_ctx, &m_wizardFields , this);
    auto createWalletSeed = new PageWalletSeed(m_ctx, &m_wizardFields, this);
    auto walletSetPasswordPage = new PageSetPassword(m_ctx, &m_wizardFields, this);
    setPage(Page_Menu, menuPage);
    setPage(Page_WalletFile, createWallet);
    setPage(Page_OpenWallet, openWalletPage);
    setPage(Page_CreateWalletSeed, createWalletSeed);
    setPage(Page_SetPasswordPage, walletSetPasswordPage);
    setPage(Page_Network, networkPage);
    setPage(Page_NetworkTor, networkTorPage);
    setPage(Page_WalletRestoreSeed, new PageWalletRestoreSeed(m_ctx, &m_wizardFields, this));
    setPage(Page_WalletRestoreKeys, new PageWalletRestoreKeys(m_ctx, &m_wizardFields, this));
    setPage(Page_SetRestoreHeight, new PageSetRestoreHeight(m_ctx, &m_wizardFields, this));
    setPage(Page_HardwareDevice, new PageHardwareDevice(m_ctx, &m_wizardFields, this));

    setStartId(startPage);

    setButtonText(QWizard::CancelButton, "Close");
    setPixmap(QWizard::WatermarkPixmap, QPixmap(":/assets/images/banners/3.png"));
    setWizardStyle(WizardStyle::ModernStyle);
    setOption(QWizard::NoBackButtonOnStartPage);

    connect(this, &QWizard::rejected, [=]{
        return QApplication::exit(1);
    });

    connect(networkTorPage, &PageNetworkTor::initialNetworkConfigured, [this](){
        emit initialNetworkConfigured();
    });

    connect(menuPage, &PageMenu::enableDarkMode, [this](bool enable){
        if (enable)
            emit skinChanged("QDarkStyle");
        else
            emit skinChanged("Native");
    });

    connect(walletSetPasswordPage, &PageSetPassword::createWallet, this, &WalletWizard::createWallet);
    connect(createWallet, &PageWalletFile::defaultWalletDirChanged, [this](const QString &walletDir){
        emit defaultWalletDirChanged(walletDir);
    });

    connect(openWalletPage, &PageOpenWallet::openWallet, [=](const QString &path){
        emit openWallet(path, "");
    });
}

void WalletWizard::createWallet() {
    auto walletPath = QString("%1/%2").arg(m_wizardFields.walletDir, m_wizardFields.walletName);

    int currentBlockHeight = 0;
    if (appData()->heights.contains(m_ctx->networkType)) {
        currentBlockHeight = appData()->heights[m_ctx->networkType];
    }

    if (m_wizardFields.mode == WizardMode::CreateWalletFromDevice) {
        int restoreHeight = currentBlockHeight;
        if (m_wizardFields.restoreHeight > 0) {
            restoreHeight = m_wizardFields.restoreHeight;
        }

        m_ctx->createWalletFromDevice(walletPath, m_wizardFields.password, restoreHeight);
        return;
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromKeys) {
        m_ctx->createWalletFromKeys(walletPath,
                                    m_wizardFields.password,
                                    m_wizardFields.address,
                                    m_wizardFields.secretViewKey,
                                    m_wizardFields.secretSpendKey,
                                    m_wizardFields.restoreHeight);
        return;
    }

    auto seed = FeatherSeed(m_ctx->networkType, QString::fromStdString(globals::coinName), m_ctx->seedLanguage, m_wizardFields.seed.split(" "));

    if (m_wizardFields.mode == WizardMode::CreateWallet) {
        qInfo() << "New wallet, setting restore height to latest blockheight: " << currentBlockHeight;
        seed.setRestoreHeight(currentBlockHeight);
    }

    if (m_wizardFields.mode == WizardMode::RestoreFromSeed && m_wizardFields.seedType == SeedType::MONERO)
        seed.setRestoreHeight(m_wizardFields.restoreHeight);

    m_ctx->createWallet(seed, walletPath, m_wizardFields.password, m_wizardFields.seedOffsetPassphrase);
}
