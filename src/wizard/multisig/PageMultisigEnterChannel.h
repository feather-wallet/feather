// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_PAGEMULTISIGENTERCHANNEL_H
#define FEATHER_PAGEMULTISIGENTERCHANNEL_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "wizard/WalletWizard.h"

namespace Ui {
    class PageMultisigEnterChannel;
}

class PageMultisigEnterChannel : public QWizardPage
{
Q_OBJECT

public:
    explicit PageMultisigEnterChannel(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:
    void registerChannel();

private:
    Ui::PageMultisigEnterChannel *ui;
    WizardFields *m_fields;

    bool m_channelRegistered = false;
};


#endif //FEATHER_PAGEMULTISIGENTERCHANNEL_H
