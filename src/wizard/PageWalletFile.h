// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_CREATEWALLET_H
#define FEATHER_CREATEWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QDir>

namespace Ui {
    class PageWalletFile;
}

class PageWalletFile : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageWalletFile(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;
    bool isComplete() const override;

private:
    QString defaultWalletName();
    bool walletPathExists(const QString &walletName);
    bool validateWidgets();

    Ui::PageWalletFile *ui;
    WizardFields *m_fields;
    bool m_validated;
};

#endif //FEATHER_CREATEWALLET_H
