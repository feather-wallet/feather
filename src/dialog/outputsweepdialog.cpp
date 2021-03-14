// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "ui_outputsweepdialog.h"
#include "outputsweepdialog.h"
#include "libwalletqt/WalletManager.h"

OutputSweepDialog::OutputSweepDialog(QWidget *parent, CoinsInfo* coin)
        : QDialog(parent)
        , ui(new Ui::OutputSweepDialog)
{
    ui->setupUi(this);

    m_amount = coin->amount();

    connect(ui->checkBox_churn, &QCheckBox::toggled, [&](bool toggled){
       ui->lineEdit_address->setEnabled(!toggled);
       ui->lineEdit_address->setText(toggled ? "Primary address" : "");
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [&](){
        m_address = ui->lineEdit_address->text();
        m_churn = ui->checkBox_churn->isChecked();
        m_outputs = ui->spinBox_numOutputs->value();
    });

    connect(ui->spinBox_numOutputs, QOverload<int>::of(&QSpinBox::valueChanged), [this](int value){
        if (value == 1) {
            ui->label_split->setText("");
            return;
        }

        QString origAmount = WalletManager::displayAmount(m_amount);
        QString splitAmount = WalletManager::displayAmount(m_amount / value);

        ui->label_split->setText(QString("%1 XMR ≈ %2x %3 XMR").arg(origAmount, QString::number(value), splitAmount));
    });
    ui->label_split->setText("");

    this->adjustSize();
}

OutputSweepDialog::~OutputSweepDialog()
{
    delete ui;
}

QString OutputSweepDialog::address() {
    return m_address;
}

bool OutputSweepDialog::churn() const {
    return m_churn;
}

int OutputSweepDialog::outputs() const {
    return m_outputs;
}