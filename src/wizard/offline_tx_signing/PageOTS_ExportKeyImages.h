// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEOTS_EXPORTKEYIMAGES_H
#define FEATHER_PAGEOTS_EXPORTKEYIMAGES_H

#include <QWizardPage>
#include "Wallet.h"
#include "OfflineTxSigningWizard.h"

namespace Ui {
    class PageOTS_Export;
}

class PageOTS_ExportKeyImages : public QWizardPage
{
Q_OBJECT

public:
    explicit PageOTS_ExportKeyImages(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields);
    void initializePage() override;
    [[nodiscard]] int nextId() const override;

private slots:
    void exportKeyImages();

private:
    void setupUR(bool all);
    
    Ui::PageOTS_Export *ui;
    Wallet *m_wallet;
    TxWizardFields *m_wizardFields;
};

#endif //FEATHER_PAGEOTS_EXPORTKEYIMAGES_H
