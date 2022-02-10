// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_RESTOREHEIGHTDIALOG_H
#define FEATHER_RESTOREHEIGHTDIALOG_H

#include <QDialog>

#include "components.h"
#include "widgets/RestoreHeightWidget.h"

class RestoreHeightDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit RestoreHeightDialog(QWidget *parent = nullptr, quint64 currentRestoreHeight = 0);
    int getHeight();

private:
    RestoreHeightWidget *m_restoreHeightWidget;
};


#endif //FEATHER_RESTOREHEIGHTDIALOG_H
