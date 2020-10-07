// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "ui_outputsweepdialog.h"
#include "outputsweepdialog.h"

OutputSweepDialog::OutputSweepDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::OutputSweepDialog)
{
    ui->setupUi(this);

    connect(ui->checkBox_churn, &QCheckBox::toggled, [&](bool toggled){
       ui->lineEdit_address->setEnabled(!toggled);
    });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [&](){
        m_address = ui->lineEdit_address->text();
        m_churn = ui->checkBox_churn->isChecked();
        m_outputs = ui->spinBox_numOutputs->value();
    });

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