// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "ui_contactsdialog.h"
#include "contactsdialog.h"

ContactsDialog::ContactsDialog(QWidget *parent, const QString &address, const QString &name)
    : QDialog(parent)
    , ui(new Ui::ContactsDialog)
{
    ui->setupUi(this);
    setMinimumWidth(400);

    ui->lineEdit_address->setText(address);
    ui->lineEdit_name->setText(name);
    if (!name.isEmpty()) {
        ui->lineEdit_name->setFocus();
    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted, [&](){
        m_address = ui->lineEdit_address->text();
        m_name = ui->lineEdit_name->text();
    });

    this->adjustSize();
}

ContactsDialog::~ContactsDialog()
{
    delete ui;
}

QString ContactsDialog::getAddress() {
    return m_address;
}

QString ContactsDialog::getName() {
    return m_name;
}