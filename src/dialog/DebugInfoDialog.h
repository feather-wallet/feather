// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_DEBUGINFODIALOG_H
#define FEATHER_DEBUGINFODIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class DebugInfoDialog;
}

class DebugInfoDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit DebugInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~DebugInfoDialog() override;

private:
    QString statusToString(Wallet::ConnectionStatus status);
    void copyToClipboad();
    void updateInfo();

    QScopedPointer<Ui::DebugInfoDialog> ui;
    QSharedPointer<AppContext> m_ctx;

    QTimer m_updateTimer;
};

#endif //FEATHER_DEBUGINFODIALOG_H
