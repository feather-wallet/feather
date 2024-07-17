// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PAGEMULTISIGSETUPKEY_H
#define PAGEMULTISIGSETUPKEY_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigSetupKey;
}

class PageMultisigSetupKey : public QWizardPage
{
    Q_OBJECT

    public:
    explicit PageMultisigSetupKey(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

    Ui::PageMultisigSetupKey *ui;
    WizardFields *m_fields;

    QString m_setupKey;
    bool m_keyGenerated = false;
};


#endif //PAGEMULTISIGSETUPKEY_H
