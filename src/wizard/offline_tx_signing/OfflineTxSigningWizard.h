// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_OFFLINETXSIGNINGWIZARD_H
#define FEATHER_OFFLINETXSIGNINGWIZARD_H

#include <QWizard>
#include <QFileDialog>

#include "Wallet.h"
#include "qrcode/scanner/QrCodeScanWidget.h"

struct TxWizardFields {
    UnsignedTransaction *utx = nullptr;
    PendingTransaction *tx = nullptr;
    std::string signedTx;
    QrCodeScanWidget *scanWidget = nullptr;
    bool readyToCommit = false;
    bool readyToSign = false;
    std::string keyImages;
};

class OfflineTxSigningWizard : public QWizard
{
    Q_OBJECT
    
public:
    enum Page {
        Page_ExportOutputs = 0,
        Page_ExportKeyImages,
        Page_ImportKeyImages,
        Page_ExportUnsignedTx,
        Page_ImportUnsignedTx,
        Page_SignTx,
        Page_ExportSignedTx,
        Page_ImportSignedTx,
        Page_ImportOffline
    };

    explicit OfflineTxSigningWizard(QWidget *parent, Wallet *wallet, PendingTransaction *tx = nullptr);
    ~OfflineTxSigningWizard() override;

    bool readyToCommit();
    bool readyToSign();
    UnsignedTransaction* unsignedTransaction();
    PendingTransaction* signedTx();
    
private:
    Wallet *m_wallet;
    TxWizardFields m_wizardFields;
};


#endif //FEATHER_OFFLINETXSIGNINGWIZARD_H
