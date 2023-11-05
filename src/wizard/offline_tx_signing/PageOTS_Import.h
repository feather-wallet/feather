// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PAGEOTS_IMPORT_H
#define FEATHER_PAGEOTS_IMPORT_H

#include <QWizardPage>
#include "Wallet.h"
#include "qrcode/scanner/QrCodeScanWidget.h"
#include "OfflineTxSigningWizard.h"

namespace Ui {
    class PageOTS_Import;
}

class PageOTS_Import : public QWizardPage
{
Q_OBJECT

public:
    explicit PageOTS_Import(QWidget *parent, Wallet *wallet, TxWizardFields *wizardFields, const QString &type, const QString &successButtonText = "Next");
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;

private slots:
    void onScanFinished(bool success);

private:
    virtual void importFromStr(const std::string &data) = 0;
    virtual void importFromFile() = 0;

protected:
    void onSuccess();
    
    Ui::PageOTS_Import *ui;
    TxWizardFields *m_wizardFields;
    QrCodeScanWidget *m_scanWidget;
    bool m_success = false;
    Wallet *m_wallet;
    QString m_type;
    QString m_successButtonText;
};

#endif //FEATHER_PAGEOTS_IMPORT_H
