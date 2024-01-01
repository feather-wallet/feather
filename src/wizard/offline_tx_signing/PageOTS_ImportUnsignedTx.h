// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEOTS_IMPORTUNSIGNEDTX_H
#define FEATHER_PAGEOTS_IMPORTUNSIGNEDTX_H

#include <QWizardPage>
#include "Wallet.h"
#include "OfflineTxSigningWizard.h"
#include "PageOTS_Import.h"

namespace Ui {
    class PageOTS_Import;
}

class PageOTS_ImportUnsignedTx : public PageOTS_Import
{
Q_OBJECT

public:
    explicit PageOTS_ImportUnsignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
    [[nodiscard]] int nextId() const override;

private slots:
    void importFromStr(const std::string &data) override;
};

#endif //FEATHER_PAGEOTS_IMPORTUNSIGNEDTX_H
