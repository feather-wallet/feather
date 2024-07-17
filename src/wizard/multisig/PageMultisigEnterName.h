// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PAGEMULTISIGENTERNAME_H
#define PAGEMULTISIGENTERNAME_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigEnterName;
}

class PageMultisigEnterName : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageMultisigEnterName(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    Ui::PageMultisigEnterName *ui;
    WizardFields *m_fields;
};

#endif //PAGEMULTISIGENTERNAME_H
