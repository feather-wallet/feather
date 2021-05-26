// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_VIEWONLYDIALOG_H
#define FEATHER_VIEWONLYDIALOG_H

#include <QDialog>
#include "appcontext.h"

namespace Ui {
    class ViewOnlyDialog;
}

class ViewOnlyDialog : public QDialog
{
Q_OBJECT

public:
    explicit ViewOnlyDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~ViewOnlyDialog() override;

private slots:
    void onWriteViewOnlyWallet();

private:
    Ui::ViewOnlyDialog *ui;
    QSharedPointer<AppContext> m_ctx;
    void copyToClipboad();
};


#endif //FEATHER_KEYSDIALOG_H
