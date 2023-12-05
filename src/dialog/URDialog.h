// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_URDIALOG_H
#define FEATHER_URDIALOG_H

#include <QDialog>

#include "components.h"

namespace Ui {
    class URDialog;
}

struct ViewOnlyDetails {
    QString address;
    QString key;
    int restoreHeight = 0;
    QString walletName;
};

class URDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit URDialog(QWidget *parent, const QString &data = "", bool scanOnly = false);
    ~URDialog() override;

    ViewOnlyDetails getViewOnlyDetails();

private:
    QScopedPointer<Ui::URDialog> ui;
    ViewOnlyDetails m_viewOnlyDetails;
};


#endif //FEATHER_URDIALOG_H
