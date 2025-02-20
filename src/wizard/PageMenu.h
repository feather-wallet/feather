// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WIZARDMENU_H
#define FEATHER_WIZARDMENU_H

#include <QWizardPage>

class WizardFields;
class WalletKeysFilesModel;

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

private:
    Ui::PageMenu *ui;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields *m_fields;
};

#endif //FEATHER_WIZARDMENU_H
