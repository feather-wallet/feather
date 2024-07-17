// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGOWNADDRESS_H
#define FEATHER_PAGEMULTISIGOWNADDRESS_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigOwnAddress;
}

class PageMultisigOwnAddress : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigOwnAddress(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    int nextId() const override;

private:
    Ui::PageMultisigOwnAddress *ui;
    WizardFields *m_fields;
};

#endif //FEATHER_PAGEMULTISIGOWNADDRESS_H
