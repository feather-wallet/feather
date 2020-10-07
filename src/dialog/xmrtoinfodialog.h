// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_XMRTOINFODIALOG_H
#define FEATHER_XMRTOINFODIALOG_H

#include <QDialog>
#include "utils/xmrto.h"

namespace Ui {
    class XmrToInfoDialog;
}

class XmrToInfoDialog : public QDialog
{
Q_OBJECT

public:
    explicit XmrToInfoDialog(XmrToOrder *oInfo, QWidget *parent = nullptr);
    ~XmrToInfoDialog() override;

private:
    Ui::XmrToInfoDialog *ui;

    XmrToOrder *m_oInfo;
};

#endif //FEATHER_XMRTOINFODIALOG_H
