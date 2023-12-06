// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef ADDRESSCHECKERINDEXDIALOG_H
#define ADDRESSCHECKERINDEXDIALOG_H

#include "components.h"
#include "Wallet.h"

namespace Ui {
    class AddressCheckerIndexDialog;
}

class AddressCheckerIndexDialog : public WindowModalDialog
{
    Q_OBJECT

    public:
    explicit AddressCheckerIndexDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~AddressCheckerIndexDialog() override;

private:
    uint32_t index();
    QString address();

    QScopedPointer<Ui::AddressCheckerIndexDialog> ui;
    Wallet *m_wallet;
};

#endif //ADDRESSCHECKERINDEXDIALOG_H
