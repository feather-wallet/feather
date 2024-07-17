// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGCREATESETUPKEY_H
#define FEATHER_PAGEMULTISIGCREATESETUPKEY_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigCreateSetupKey;
}

class PageMultisigCreateSetupKey : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigCreateSetupKey(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    [[nodiscard]] int nextId() const override;

private:
    Ui::PageMultisigCreateSetupKey *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGCREATESETUPKEY_H
