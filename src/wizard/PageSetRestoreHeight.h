// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_PAGESETRESTOREHEIGHT_H
#define FEATHER_PAGESETRESTOREHEIGHT_H

#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"
#include "WalletWizard.h"

namespace Ui {
    class PageSetRestoreHeight;
}

class PageSetRestoreHeight : public QWizardPage
{
Q_OBJECT

public:
    explicit PageSetRestoreHeight(AppContext *ctx, WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private slots:
    void onCreationDateEdited();
    void onRestoreHeightEdited();

private:
    void showScanWarning(const QDateTime &date);
    void showWalletAgeWarning(const QDateTime &date);

    AppContext *m_ctx;
    WizardFields *m_fields;
    Ui::PageSetRestoreHeight *ui;
};

#endif //FEATHER_PAGESETRESTOREHEIGHT_H
