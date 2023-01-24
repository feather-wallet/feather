// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H
#define FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H

#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"
#include "WalletWizard.h"

namespace Ui {
    class PageSetSubaddressLookahead;
}

class PageSetSubaddressLookahead : public QWizardPage
{
Q_OBJECT

public:
    explicit  PageSetSubaddressLookahead(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    Ui::PageSetSubaddressLookahead *ui;

    WizardFields *m_fields;
};


#endif //FEATHER_PAGESETSUBADDRESSLOOKAHEAD_H
