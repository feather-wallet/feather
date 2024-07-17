// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGSIGNERINFO_H
#define FEATHER_PAGEMULTISIGSIGNERINFO_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>
#include <QLineEdit>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigSignerInfo;
}

class PageMultisigSignerInfo : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigSignerInfo(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:

private:
    Ui::PageMultisigSignerInfo *ui;
    WizardFields *m_fields;

    QVector<QPair<QLineEdit*, QLineEdit*>> m_signerInfo;
};


#endif //FEATHER_PAGEMULTISIGSIGNERINFO_H
