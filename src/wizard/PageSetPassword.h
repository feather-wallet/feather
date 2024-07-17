// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PASSWORD_H
#define FEATHER_PASSWORD_H

#include <QWizardPage>
#include <QWidget>

#include "WalletWizard.h"

namespace Ui {
    class PageSetPassword;
}

class PageSetPassword : public QWizardPage
{
Q_OBJECT

public:
    explicit PageSetPassword(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

signals:
    void createWallet();
    void createMultisigWallet();

private:
    Ui::PageSetPassword *ui;

    bool m_walletCreated = false;
    WizardFields *m_fields;
};


#endif //FEATHER_PASSWORD_H
