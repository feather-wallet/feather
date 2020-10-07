// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_DEBUGINFODIALOG_H
#define FEATHER_DEBUGINFODIALOG_H

#include <QDialog>
#include "appcontext.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class DebugInfoDialog;
}

class DebugInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DebugInfoDialog(AppContext *ctx, QWidget *parent = nullptr);
    ~DebugInfoDialog() override;

private:
    QString statusToString(Wallet::ConnectionStatus status);
    void copyToClipboad();

    Ui::DebugInfoDialog *ui;
};

#endif //FEATHER_DEBUGINFODIALOG_H
