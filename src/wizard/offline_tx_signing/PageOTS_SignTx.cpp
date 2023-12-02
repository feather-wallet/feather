// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "PageOTS_SignTx.h"

PageOTS_SignTx::PageOTS_SignTx(QWidget *parent)
        : QWizardPage(parent)
{
    // Serves no purpose other than to close the wizard.
}

int PageOTS_SignTx::nextId() const {
    return -1;
}

void PageOTS_SignTx::initializePage() {
    QTimer::singleShot(1, [this]{
        this->wizard()->button(QWizard::FinishButton)->click();
    });
}