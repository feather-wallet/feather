// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEOTS_EXPORTSIGNEDTX_H
#define FEATHER_PAGEOTS_EXPORTSIGNEDTX_H

#include <QWizardPage>
#include "Wallet.h"
#include "OfflineTxSigningWizard.h"

namespace Ui {
    class PageOTS_Export;
}

class PageOTS_ExportSignedTx : public QWizardPage
{
Q_OBJECT

public:
    explicit PageOTS_ExportSignedTx(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
    void initializePage() override;
    [[nodiscard]] int nextId() const override;

private slots:
    void exportSignedTx();

private:
    Ui::PageOTS_Export *ui;
    Wallet *m_wallet;
    TxWizardFields *m_wizardFields;
};

#endif //FEATHER_PAGEOTS_EXPORTSIGNEDTX_H
