// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef HISTORYEXPORTDIALOG_H
#define HISTORYEXPORTDIALOG_H

#include "components.h"

namespace Ui {
class HistoryExportDialog;
}

class Wallet;
class HistoryExportDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit HistoryExportDialog(Wallet *wallet, QWidget *parent);
    ~HistoryExportDialog() override;

private:
    void setEverything();
    void exportHistory();

    QScopedPointer<Ui::HistoryExportDialog> ui;
    Wallet *m_wallet;
};


#endif //HISTORYEXPORTDIALOG_H
