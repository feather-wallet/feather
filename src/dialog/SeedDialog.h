// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_SEEDDIALOG_H
#define FEATHER_SEEDDIALOG_H

#include <QDialog>

#include "components.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class SeedDialog;
}

class SeedDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit SeedDialog(Wallet *wallet, QWidget *parent = nullptr);
    ~SeedDialog() override;

private:
    void setSeed(const QString &seed);
    void setMultisigSeed(const QString &seed);

    QScopedPointer<Ui::SeedDialog> ui;
    Wallet *m_wallet;
};


#endif //FEATHER_SEEDDIALOG_H
