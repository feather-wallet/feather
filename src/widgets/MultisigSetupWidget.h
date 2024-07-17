// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_MULTISIGSETUPWIDGET_H
#define FEATHER_MULTISIGSETUPWIDGET_H

#include <QItemSelection>
#include <QTreeView>
#include <QWidget>

#include "model/NodeModel.h"
#include "utils/nodes.h"

namespace Ui {
    class MultisigSetupWidget;
}


class MultisigSetupWidget : public QWidget
{
Q_OBJECT

public:
    explicit MultisigSetupWidget(QWidget *parent = nullptr);
    ~MultisigSetupWidget();

    QScopedPointer<Ui::MultisigSetupWidget> ui;
};

#endif //FEATHER_MULTISIGSETUPWIDGET_H
