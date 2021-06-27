// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_LOCALMONEROINFODIALOG_H
#define FEATHER_LOCALMONEROINFODIALOG_H

#include <QDialog>
#include <QLabel>
#include "model/LocalMoneroModel.h"

namespace Ui {
    class LocalMoneroInfoDialog;
}

class LocalMoneroInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LocalMoneroInfoDialog(QWidget *parent, LocalMoneroModel *model, int row);
    ~LocalMoneroInfoDialog() override;

private slots:
    void onGoToOffer();

private:
    void setLabelText(QLabel *label, LocalMoneroModel::Column column);

    QScopedPointer<Ui::LocalMoneroInfoDialog> ui;
    LocalMoneroModel *m_model;
    int m_row;
};


#endif //FEATHER_INFODIALOG_H
