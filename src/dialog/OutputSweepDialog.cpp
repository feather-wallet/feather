// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "OutputSweepDialog.h"
#include "ui_OutputSweepDialog.h"

#include "libwalletqt/WalletManager.h"

OutputSweepDialog::OutputSweepDialog(QWidget *parent, quint64 amount)
        : WindowModalDialog(parent)
        , ui(new Ui::OutputSweepDialog)
        , m_amount(amount)
{
    ui->setupUi(this);

    connect(ui->checkBox_churn, &QCheckBox::toggled, [&](bool toggled){
       ui->lineEdit_address->setEnabled(!toggled);
       ui->lineEdit_address->setText(toggled ? "This account" : "");
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [&](){
        m_address = ui->lineEdit_address->text();
        m_churn = ui->checkBox_churn->isChecked();
        m_outputs = ui->spinBox_numOutputs->value();
        m_feeLevel = ui->combo_feePriority->currentIndex();
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

QString OutputSweepDialog::address() {
    return m_address;
}

bool OutputSweepDialog::churn() const {
    return m_churn;
}

int OutputSweepDialog::outputs() const {
    return m_outputs;
}

int OutputSweepDialog::feeLevel() const {
    return m_feeLevel;
}

OutputSweepDialog::~OutputSweepDialog() = default;