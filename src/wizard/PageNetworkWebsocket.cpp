// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageNetworkWebsocket.h"
#include "ui_PageNetworkWebsocket.h"
#include "WalletWizard.h"

PageNetworkWebsocket::PageNetworkWebsocket(QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageNetworkWebsocket)
{
    ui->setupUi(this);

    this->setCommitPage(true);
    this->setButtonText(QWizard::CommitButton, "Next");
}

int PageNetworkWebsocket::nextId() const {
    return WalletWizard::Page_Menu;
}

bool PageNetworkWebsocket::validatePage() {
    bool disabled = ui->btn_disable->isChecked();
    conf()->set(Config::disableWebsocket, disabled);

    emit initialNetworkConfigured();

    return true;
}