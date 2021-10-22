// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXCONFADVDIALOG_H
#define FEATHER_TXCONFADVDIALOG_H

#include <QDialog>
#include <QMenu>
#include <QStandardItemModel>
#include <QTextCharFormat>

#include "appcontext.h"
#include "components.h"
#include "libwalletqt/PendingTransaction.h"

namespace Ui {
    class TxConfAdvDialog;
}

class TxConfAdvDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit TxConfAdvDialog(QSharedPointer<AppContext> ctx, const QString &description, QWidget *parent = nullptr);
    ~TxConfAdvDialog() override;

    void setTransaction(PendingTransaction *tx, bool isSigned = true); // #TODO: have libwallet return a UnsignedTransaction, this is just dumb
    void setUnsignedTransaction(UnsignedTransaction *utx);

private:
    void setupConstructionData(ConstructionInfo *ci);
    void signTransaction();
    void broadcastTransaction();
    void closeDialog();
    void setAmounts(quint64 amount, quint64 fee);

    void unsignedCopy();
    void unsignedQrCode();
    void unsignedSaveFile();

    void signedCopy();
    void signedQrCode();
    void signedSaveFile();

    void txKeyCopy();

    QScopedPointer<Ui::TxConfAdvDialog> ui;
    QSharedPointer<AppContext> m_ctx;
    PendingTransaction *m_tx = nullptr;
    UnsignedTransaction *m_utx = nullptr;
    QMenu *m_exportUnsignedMenu;
    QMenu *m_exportSignedMenu;
    QMenu *m_exportTxKeyMenu;
    QString m_txid;
};

#endif //FEATHER_TXCONFADVDIALOG_H
