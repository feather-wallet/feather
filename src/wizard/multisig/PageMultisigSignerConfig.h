// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PAGEMULTISIGSIGNERCONFIG_H
#define PAGEMULTISIGSIGNERCONFIG_H

#include <QWizardPage>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigSignerConfig;
}

class PageMultisigSignerConfig : public QWizardPage
{
    Q_OBJECT

    public:
    explicit PageMultisigSignerConfig(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

    Ui::PageMultisigSignerConfig *ui;
    WizardFields *m_fields;
};

#endif //PAGEMULTISIGSIGNERCONFIG_H
