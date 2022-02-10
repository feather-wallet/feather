// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_VIEWONLYDIALOG_H
#define FEATHER_VIEWONLYDIALOG_H

#include <QDialog>

#include "appcontext.h"
#include "components.h"

namespace Ui {
    class ViewOnlyDialog;
}

class ViewOnlyDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit ViewOnlyDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~ViewOnlyDialog() override;

private slots:
    void onWriteViewOnlyWallet();

private:
    void copyToClipboad();

    QScopedPointer<Ui::ViewOnlyDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};


#endif //FEATHER_KEYSDIALOG_H
