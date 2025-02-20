// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PAGEOTS_SIGNTX_H
#define FEATHER_PAGEOTS_SIGNTX_H

#include <QWizardPage>
#include <QCheckBox>
#include "Wallet.h"
#include "OfflineTxSigningWizard.h"

class PageOTS_SignTx : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOTS_SignTx(QWidget *parent);
    void initializePage() override;
    [[nodiscard]] int nextId() const override;
};


#endif //FEATHER_PAGEOTS_SIGNTX_H
