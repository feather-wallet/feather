// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_OUTPUTSWEEPDIALOG_H
#define FEATHER_OUTPUTSWEEPDIALOG_H

#include <QDialog>
#include "libwalletqt/CoinsInfo.h"

namespace Ui {
    class OutputSweepDialog;
}

class OutputSweepDialog : public QDialog
{
Q_OBJECT

public:
    explicit OutputSweepDialog(QWidget *parent, CoinsInfo* coin);
    ~OutputSweepDialog() override;

    QString address();
    bool churn() const;
    int outputs() const;

private:
    Ui::OutputSweepDialog *ui;

    uint64_t m_amount;

    QString m_address;
    bool m_churn;
    int m_outputs;
};


#endif //FEATHER_OUTPUTSWEEPDIALOG_H
