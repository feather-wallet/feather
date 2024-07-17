// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGSETUPDEBUG_H
#define FEATHER_PAGEMULTISIGSETUPDEBUG_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "wizard/WalletWizard.h"
#include "MMSWidget.h"

namespace Ui {
    class PageMultisigSetupDebug;
}

class PageMultisigSetupDebug : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigSetupDebug(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

signals:
    void showWallet(Wallet *wallet);

private slots:
    void onMultisigWalletCreated(const QString &address);

private:
    void setStatus(const QString &status);

    Ui::PageMultisigSetupDebug *ui;
    WizardFields *m_fields;
    MMSWidget *m_mmsWidget;
};


#endif //FEATHER_PAGEMULTISIGSETUPDEBUG_H
