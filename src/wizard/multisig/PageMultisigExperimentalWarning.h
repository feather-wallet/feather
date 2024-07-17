// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGEXPERIMENTALWARNING_H
#define FEATHER_PAGEMULTISIGEXPERIMENTALWARNING_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigExperimentalWarning;
}

class PageMultisigExperimentalWarning : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigExperimentalWarning(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

    Ui::PageMultisigExperimentalWarning *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGEXPERIMENTALWARNING_H
