// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "RestoreHeightDialog.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>

RestoreHeightDialog::RestoreHeightDialog(QWidget *parent, quint64 currentRestoreHeight)
    : WindowModalDialog(parent)
    , m_restoreHeightWidget(new RestoreHeightWidget(this))
{
    auto *layout = new QVBoxLayout(this);

    auto *label = new QLabel(this);
    label->setText("Enter a wallet creation date or restore height.");

    auto *buttonBox = new QDialogButtonBox(this);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    layout->addWidget(label);
    layout->addWidget(m_restoreHeightWidget);
    layout->addWidget(buttonBox);

    this->setLayout(layout);

    if (currentRestoreHeight) {
        m_restoreHeightWidget->setHeight(currentRestoreHeight);
    }

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->adjustSize();
}

int RestoreHeightDialog::getHeight() {
    return m_restoreHeightWidget->getHeight();
}