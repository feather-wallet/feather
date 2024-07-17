// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEKEYTYPE_H
#define FEATHER_PAGEKEYTYPE_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "WalletWizard.h"

namespace Ui {
    class PageKeyType;
}

class PageKeyType : public QWizardPage
{
Q_OBJECT

public:
    explicit PageKeyType(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    Ui::PageKeyType *ui;
    WizardFields *m_fields;
};

#endif //FEATHER_PAGEKEYTYPE_H
