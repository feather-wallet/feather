// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_VIEWONLYDIALOG_H
#define FEATHER_VIEWONLYDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class ViewOnlyDialog;
}

class ViewOnlyDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit ViewOnlyDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~ViewOnlyDialog() override;

private slots:
    void onWriteViewOnlyWallet();

private:
    QString toString();
    QString toJsonString();
    void copyToClipboard();

    QScopedPointer<Ui::ViewOnlyDialog> ui;
    Wallet *m_wallet;
};


#endif //FEATHER_KEYSDIALOG_H
