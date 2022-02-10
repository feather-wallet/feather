// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_CONTACTSDIALOG_H
#define FEATHER_CONTACTSDIALOG_H

#include <QDialog>

#include "components.h"

namespace Ui {
    class ContactsDialog;
}

class ContactsDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit ContactsDialog(QWidget *parent = nullptr, const QString &address = "", const QString &name = "");
    ~ContactsDialog() override;

    QString getAddress();
    QString getName();

private:
    QScopedPointer<Ui::ContactsDialog> ui;

    QString m_address;
    QString m_name;
};

#endif //FEATHER_CONTACTSDIALOG_H
