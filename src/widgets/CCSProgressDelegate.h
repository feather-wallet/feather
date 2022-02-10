// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_CSSPROGRESSDELEGATE_H
#define FEATHER_CSSPROGRESSDELEGATE_H

#include <QStyledItemDelegate>

#include "model/CCSModel.h"

class CCSProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit CCSProgressDelegate(CCSModel *model, QWidget *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    CCSModel *m_model;
};


#endif //FEATHER_CSSPROGRESSDELEGATE_H
