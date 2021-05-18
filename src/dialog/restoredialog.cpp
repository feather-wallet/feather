// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "restoredialog.h"
#include "ui_restoredialog.h"

#include "constants.h"

RestoreDialog::RestoreDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::RestoreDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);
    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &RestoreDialog::accepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &RestoreDialog::rejected);

    if(constants::networkType == NetworkType::Type::TESTNET) {
        ui->restoreHeightWidget->hideSlider();
    } else {
        // load restoreHeight lookup db
        ui->restoreHeightWidget->initRestoreHeights(appData()->restoreHeights[constants::networkType]);
    }
}

int RestoreDialog::getHeight() {
    return ui->restoreHeightWidget->getHeight();
}

void RestoreDialog::initRestoreHeights(RestoreHeightLookup *lookup) {
    ui->restoreHeightWidget->initRestoreHeights(lookup);
}

RestoreDialog::~RestoreDialog() {
    delete ui;
}

