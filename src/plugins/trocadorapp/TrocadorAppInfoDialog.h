// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TROCADORAPPINFODIALOG_H
#define FEATHER_TROCADORAPPINFODIALOG_H

#include <QDialog>
#include <QLabel>

#include "TrocadorAppApi.h"

#include "components.h"
#include "TrocadorAppModel.h"

namespace Ui {
    class TrocadorAppInfoDialog;
}

class TrocadorAppInfoDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit TrocadorAppInfoDialog(QWidget *parent, TrocadorAppModel *model, int row);
    ~TrocadorAppInfoDialog() override;

private slots:
    void onGoToOffer();

private:
    void setLabelText(QLabel *label, TrocadorAppModel::Column column);

    QScopedPointer<Ui::TrocadorAppInfoDialog> ui;
    TrocadorAppModel *m_model;
    int m_row;
    TrocadorAppApi *m_api;
    Networking *m_network;
};


#endif //FEATHER_INFODIALOG_H
