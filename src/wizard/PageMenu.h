// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WIZARDMENU_H
#define FEATHER_WIZARDMENU_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"

namespace Ui {
    class PageMenu;
}

class PageMenu : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageMenu(WizardFields *fields, WalletKeysFilesModel *wallets, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void enableDarkMode(bool enable);

private:
    Ui::PageMenu *ui;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields *m_fields;
};

#endif //FEATHER_WIZARDMENU_H
