// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WIZARDVIEWONLY_H
#define FEATHER_WIZARDVIEWONLY_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "appcontext.h"

namespace Ui {
    class PageWalletRestoreKeys;
}

class PageWalletRestoreKeys : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageWalletRestoreKeys(AppContext *ctx, WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

private:
    void resetWidgets();

    AppContext *m_ctx;
    WizardFields *m_fields;
    Ui::PageWalletRestoreKeys *ui;
};

#endif
