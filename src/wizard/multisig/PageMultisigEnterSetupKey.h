// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGENTERSETUPKEY_H
#define FEATHER_PAGEMULTISIGENTERSETUPKEY_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigEnterSetupKey;
}

class PageMultisigEnterSetupKey : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigEnterSetupKey(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:
    void checkSetupKey(const QString &setupKey);

private:
    Ui::PageMultisigEnterSetupKey *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGENTERSETUPKEY_H
