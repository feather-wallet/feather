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

    this->setCommitPage(true);
    this->setButtonText(QWizard::CommitButton, "Next");

    QPixmap iconAllTorExceptNode(":/assets/images/securityLevelStandard.png");
    QPixmap iconAllTorExceptInitSync(":/assets/images/securityLevelSafer.png");
    QPixmap iconAllTor(":/assets/images/securityLevelSafest.png");
    ui->icon_allTorExceptNode->setPixmap(iconAllTorExceptNode.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_allTorExceptInitSync->setPixmap(iconAllTorExceptInitSync.scaledToHeight(16, Qt::SmoothTransformation));
    ui->icon_allTor->setPixmap(iconAllTor.scaledToHeight(16, Qt::SmoothTransformation));

    connect(ui->radio_configureManually, &QRadioButton::toggled, [this](bool checked){
        ui->frame_privacyLevel->setVisible(checked);
        this->adjustSize();
        this->updateGeometry();
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

void PageNetworkTor::initializePage() {
    // Fuck you Qt. No squish.
    QTimer::singleShot(1, [this]{
        ui->frame_privacyLevel->setVisible(false);
    });
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