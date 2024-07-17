//
// Created by user on 3/2/24.
//

#ifndef FEATHER_PAGERECOVERWALLET_H
#define FEATHER_PAGERECOVERWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "WalletWizard.h"

namespace Ui {
    class PageRecoverWallet;
}

class PageRecoverWallet : public QWizardPage
{
Q_OBJECT

public:
    explicit PageRecoverWallet(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    int nextId() const override;

private:
    Ui::PageRecoverWallet *ui;
    WizardFields *m_fields;
};


#endif //FEATHER_PAGERECOVERWALLET_H
