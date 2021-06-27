// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_RESTOREHEIGHTDIALOG_H
#define FEATHER_RESTOREHEIGHTDIALOG_H

#include <QDialog>

#include "widgets/RestoreHeightWidget.h"

class RestoreHeightDialog : public QDialog
{
Q_OBJECT

public:
    explicit RestoreHeightDialog(QWidget *parent = nullptr, quint64 currentRestoreHeight = 0);
    int getHeight();

private:
    RestoreHeightWidget *m_restoreHeightWidget;
};


#endif //FEATHER_RESTOREHEIGHTDIALOG_H
