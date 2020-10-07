// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_CONTACTSDIALOG_H
#define FEATHER_CONTACTSDIALOG_H

#include <QDialog>

namespace Ui {
    class ContactsDialog;
}

class ContactsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContactsDialog(QWidget *parent = nullptr);
    ~ContactsDialog() override;

    QString getAddress();
    QString getName();

private:
    Ui::ContactsDialog *ui;

    QString m_address;
    QString m_name;
};

#endif //FEATHER_CONTACTSDIALOG_H
