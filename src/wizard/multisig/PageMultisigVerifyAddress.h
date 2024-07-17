// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGVERIFYADDRESS_H
#define FEATHER_PAGEMULTISIGVERIFYADDRESS_H


#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigVerifyAddress;
}

class PageMultisigVerifyAddress : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigVerifyAddress(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:

private:
    Ui::PageMultisigVerifyAddress *ui;
    WizardFields *m_fields;
    QString m_address;
};


#endif //FEATHER_PAGEMULTISIGVERIFYADDRESS_H
