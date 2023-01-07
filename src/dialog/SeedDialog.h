// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_SEEDDIALOG_H
#define FEATHER_SEEDDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"

namespace Ui {
    class SeedDialog;
}

class SeedDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit SeedDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~SeedDialog() override;

private:
    void setSeed(const QString &seed);

    QScopedPointer<Ui::SeedDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};


#endif //FEATHER_SEEDDIALOG_H
