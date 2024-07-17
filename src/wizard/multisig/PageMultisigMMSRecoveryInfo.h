// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGMMSRECOVERYINFO_H
#define FEATHER_PAGEMULTISIGMMSRECOVERYINFO_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigMMSRecoveryInfo;
}

class PageMultisigMMSRecoveryInfo : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigMMSRecoveryInfo(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

signals:
    void showWallet(Wallet *wallet);

private:
    Ui::PageMultisigMMSRecoveryInfo *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGMMSRECOVERYINFO_H
