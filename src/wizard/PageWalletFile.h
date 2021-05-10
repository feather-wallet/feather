// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_CREATEWALLET_H
#define FEATHER_CREATEWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

#include "appcontext.h"

namespace Ui {
    class PageWalletFile;
}

class PageWalletFile : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageWalletFile(AppContext *ctx, WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

signals:
    void defaultWalletDirChanged(QString walletDir);

private:
    QString defaultWalletName();
    bool walletPathExists(const QString &walletName);
    bool validateWidgets();

    Ui::PageWalletFile *ui;
    AppContext *m_ctx;
    WizardFields *m_fields;
    bool m_validated;
};

#endif //FEATHER_CREATEWALLET_H
