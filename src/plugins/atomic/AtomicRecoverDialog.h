// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICRECOVERDIALOG_H
#define FEATHER_ATOMICRECOVERDIALOG_H

#include <QDialog>
#include "components.h"
#include "AtomicSwap.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AtomicRecoverDialog; }
QT_END_NAMESPACE

class AtomicRecoverDialog : public WindowModalDialog {
Q_OBJECT

public:
    explicit AtomicRecoverDialog(QWidget *parent = nullptr);
    bool historyEmpty();
    void appendHistory(QString entry);
    ~AtomicRecoverDialog() override;

private slots:
    void updateBtn(const QModelIndex &index);
private:
    Ui::AtomicRecoverDialog *ui = nullptr;
    AtomicSwap *swapDialog = nullptr;

};


#endif //FEATHER_ATOMICRECOVERDIALOG_H
