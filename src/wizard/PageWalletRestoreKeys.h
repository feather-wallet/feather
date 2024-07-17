// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WIZARDVIEWONLY_H
#define FEATHER_WIZARDVIEWONLY_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "WalletWizard.h"

namespace Ui {
    class PageWalletRestoreKeys;
}

class PageWalletRestoreKeys : public QWizardPage
{
    Q_OBJECT

    enum walletType {
        ViewOnly = 0,
        Spendable = 1,
        Spendable_Nondeterministic = 2,
        Multisig = 3,
    };

public:
    explicit PageWalletRestoreKeys(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    void showInputLines();

private:
    void onOptionsClicked();
    int walletType();

    Ui::PageWalletRestoreKeys *ui;
    WizardFields *m_fields;
};

#endif
