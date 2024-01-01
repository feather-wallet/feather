// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEOTS_IMPORTKEYIMAGES_H
#define FEATHER_PAGEOTS_IMPORTKEYIMAGES_H

#include <QWizardPage>
#include "Wallet.h"
#include "qrcode/scanner/QrCodeScanWidget.h"
#include "OfflineTxSigningWizard.h"
#include "PageOTS_Import.h"

namespace Ui {
    class PageOTS_Import;
}

class PageOTS_ImportKeyImages : public PageOTS_Import
{
    Q_OBJECT

public:
    explicit PageOTS_ImportKeyImages(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
    int nextId() const override;

private slots:
    void importFromStr(const std::string &data) override;
    
private:
    void onSuccess();
    bool proceed();
};

#endif //FEATHER_PAGEOTS_IMPORTKEYIMAGES_H
