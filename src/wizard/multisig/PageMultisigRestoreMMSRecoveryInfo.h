// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGRESTOREMMSRECOVERYINFO_H
#define FEATHER_PAGEMULTISIGRESTOREMMSRECOVERYINFO_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigRestoreMMSRecoveryInfo;
}

class PageMultisigRestoreMMSRecoveryInfo : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigRestoreMMSRecoveryInfo(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    Ui::PageMultisigRestoreMMSRecoveryInfo *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGRESTOREMMSRECOVERYINFO_H
