// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "PageNetworkTor.h"
#include "ui_PageNetworkTor.h"
#include "WalletWizard.h"

PageNetworkTor::PageNetworkTor(AppContext *ctx, QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PageNetworkTor)
    , m_ctx(ctx)
{
    ui->setupUi(this);

    QPixmap iconAllTorExceptNode(":/assets/images/securityLevelStandardWhite.png");
    QPixmap iconAllTorExceptInitSync(":/assets/images/securityLevelSaferWhite.png");
    QPixmap iconAllTor(":/assets/images/securityLevelSafestWhite.png");
    ui->icon_allTorExceptNode->setPixmap(iconAllTorExceptNode.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_allTorExceptInitSync->setPixmap(iconAllTorExceptInitSync.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_allTor->setPixmap(iconAllTor.scaledToHeight(16, Qt::SmoothTransformation));

    ui->frame_privacyLevel->setVisible(false);
    connect(ui->radio_configureManually, &QRadioButton::toggled, [this](bool checked){
        ui->frame_privacyLevel->setVisible(checked);
    });

    ui->btnGroup_privacyLevel->setId(ui->radio_allTorExceptNode, Config::allTorExceptNode);
    ui->btnGroup_privacyLevel->setId(ui->radio_allTorExceptInitSync, Config::allTorExceptInitSync);
    ui->btnGroup_privacyLevel->setId(ui->radio_allTor, Config::allTor);

    int privacyLevel = config()->get(Config::torPrivacyLevel).toInt();
    auto button = ui->btnGroup_privacyLevel->button(privacyLevel);
    if (button) {
        button->setChecked(true);
    }
}

int PageNetworkTor::nextId() const {
    return WalletWizard::Page_Menu;
}

bool PageNetworkTor::validatePage() {
    int id = ui->btnGroup_privacyLevel->checkedId();
    config()->set(Config::torPrivacyLevel, id);

    emit initialNetworkConfigured();

    return true;
}