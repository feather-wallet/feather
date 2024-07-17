// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGSEED_H
#define FEATHER_PAGEMULTISIGSEED_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigSeed;
}

class PageMultisigSeed : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigSeed(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    void copySeed();

    Ui::PageMultisigSeed *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGEMULTISIGSEED_H
