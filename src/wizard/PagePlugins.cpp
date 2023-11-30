// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PagePlugins.h"
#include "ui_PagePlugins.h"

#include "WalletWizard.h"

PagePlugins::PagePlugins(QWidget *parent)
    : QWizardPage(parent)
    , ui(new Ui::PagePlugins)
{
    ui->setupUi(this);
}

int PagePlugins::nextId() const {
    return WalletWizard::Page_Menu;
}

bool PagePlugins::validatePage() {
    return true;
}

bool PagePlugins::isComplete() const {
    return true;
}