// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "CCSProgressDelegate.h"

#include <QApplication>

CCSProgressDelegate::CCSProgressDelegate(CCSModel *model, QWidget *parent)
        : QStyledItemDelegate(parent)
        , m_model(model)
{
}

void CCSProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const {

    if (index.column() != CCSModel::Progress) {
        QStyledItemDelegate::paint(painter, option, index);
        return;
    }

    QStyleOptionProgressBar progressBarOption;
    progressBarOption.state = QStyle::State_Enabled;
    progressBarOption.direction = QApplication::layoutDirection();
    progressBarOption.rect = option.rect;
    progressBarOption.fontMetrics = QApplication::fontMetrics();
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    progressBarOption.textAlignment = Qt::AlignCenter;
    progressBarOption.textVisible = true;

    QSharedPointer<CCSEntry> entry = m_model->entry(index.row());
    auto target = QString("%1/%2 XMR").arg(entry->raised_amount).arg(entry->target_amount);
    auto progress = (int)entry->percentage_funded;
    progressBarOption.progress = progress < 0 ? 0 : progress;
    progressBarOption.text = target;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);  // Draw the progress bar onto the view.
}