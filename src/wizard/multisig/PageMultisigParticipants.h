// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef PAGEMULTISIGPARTICIPANTS_H
#define PAGEMULTISIGPARTICIPANTS_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigParticipants;
}

class PageMultisigParticipants : public QWizardPage
{
    Q_OBJECT

    public:
    explicit PageMultisigParticipants(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

    Ui::PageMultisigParticipants *ui;
    WizardFields *m_fields;
};

#endif //PAGEMULTISIGPARTICIPANTS_H
