// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAGEOTS_EXPORTUNSIGNEDTX_H
#define FEATHER_PAGEOTS_EXPORTUNSIGNEDTX_H

#include <QWizardPage>
#include "Wallet.h"
#include "PendingTransaction.h"

namespace Ui {
    class PageOTS_Export;
}

class PageOTS_ExportUnsignedTx : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOTS_ExportUnsignedTx(QWidget *parent, Wallet *wallet, PendingTransaction *tx = nullptr);
    void initializePage() override;
    int nextId() const override;

private slots:
    void exportUnsignedTx();

private:
    Ui::PageOTS_Export *ui;
    Wallet *m_wallet;
    PendingTransaction *m_tx;
};

#endif //FEATHER_PAGEOTS_EXPORTUNSIGNEDTX_H
