// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGSETUPWALLET_H
#define FEATHER_PAGEMULTISIGSETUPWALLET_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigSetupWallet;
}

class PageMultisigSetupWallet : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigSetupWallet(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:
    void setStatus(const QString &status, bool finished = false);
    void updateSignerConfig();
    void onWalletCreated();

private:
    Ui::PageMultisigSetupWallet *ui;
    WizardFields *m_fields;
    bool m_created = false;
};


#endif //FEATHER_PAGEMULTISIGSETUPWALLET_H
