// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_DEBUGINFODIALOG_H
#define FEATHER_DEBUGINFODIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"
#include "utils/nodes.h"

namespace Ui {
    class DebugInfoDialog;
}

class DebugInfoDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit DebugInfoDialog(Wallet *wallet, Nodes *nodes, QWidget *parent = nullptr);
    ~DebugInfoDialog() override;

private:
    QString statusToString(Wallet::ConnectionStatus status);
    void copyToClipboard();
    void updateInfo();

    QScopedPointer<Ui::DebugInfoDialog> ui;
    Wallet *m_wallet;
    Nodes *m_nodes;

    QTimer m_updateTimer;
};

#endif //FEATHER_DEBUGINFODIALOG_H
