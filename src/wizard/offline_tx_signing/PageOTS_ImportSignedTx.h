// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAGEOTS_IMPORTSIGNEDTX_H
#define FEATHER_PAGEOTS_IMPORTSIGNEDTX_H

#include <QWizardPage>
#include "Wallet.h"
#include "qrcode/scanner/QrCodeScanWidget.h"
#include "OfflineTxSigningWizard.h"
#include "PageOTS_Import.h"

namespace Ui {
    class PageOTS_Import;
}

class PageOTS_ImportSignedTx : public PageOTS_Import
{
Q_OBJECT

public:
    explicit PageOTS_ImportSignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
//    void initializePage() override;
    int nextId() const override;

private slots:
    void importFromStr(const std::string &data) override;
    void importFromFile() override;

private:
    bool validatePage() override;
};

#endif //FEATHER_PAGEOTS_IMPORTSIGNEDTX_H
