// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGRESTORESEED_H
#define FEATHER_PAGEMULTISIGRESTORESEED_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigRestoreSeed;
}

class PageMultisigRestoreSeed : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageMultisigRestoreSeed(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    void onOptionsClicked();

    Ui::PageMultisigRestoreSeed *ui;
    WizardFields *m_fields;
};

#endif //FEATHER_PAGEMULTISIGRESTORESEED_H
