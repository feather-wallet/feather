// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef RESTOREDIALOG_H
#define RESTOREDIALOG_H

#include <QPushButton>
#include <QDialogButtonBox>
#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractButton>

#include "utils/RestoreHeightLookup.h"
#include "appcontext.h"

namespace Ui {
    class RestoreDialog;
}

class RestoreDialog : public QDialog
{
Q_OBJECT

public:
    explicit RestoreDialog(AppContext *ctx, QWidget *parent = nullptr);
    void initRestoreHeights(RestoreHeightLookup *lookup);
    int getHeight();
    ~RestoreDialog() override;

signals:
    void accepted();
    void rejected();

private:
    AppContext *m_ctx;
    Ui::RestoreDialog *ui;
};

#endif // RESTOREDIALOG_H
