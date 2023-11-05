// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAGEOTS_IMPORTOFFLINE_H
#define FEATHER_PAGEOTS_IMPORTOFFLINE_H

#include <QWizardPage>
#include "Wallet.h"
#include "qrcode/scanner/QrCodeScanWidget.h"
#include "OfflineTxSigningWizard.h"
#include "PageOTS_Import.h"

namespace Ui {
    class PageOTS_Import;
}

class PageOTS_ImportOffline : public PageOTS_Import
{
Q_OBJECT

enum ImportType {
    OUTPUTS = 0,
    UNSIGNED_TX
};

public:
    explicit PageOTS_ImportOffline(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
    int nextId() const override;

private slots:
    void importFromStr(const std::string &data) override;
    void importFromFile() override;

private:
    bool isOutputs(const std::string &data);
    bool isUnsignedTransaction(const std::string &data);

    ImportType m_importType;
};

#endif //FEATHER_PAGEOTS_IMPORT_H
