// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SUBADDRESSVIEW_H
#define FEATHER_SUBADDRESSVIEW_H

#include <QTreeView>
#include <QKeyEvent>
#include <QClipboard>

class SubaddressView : public QTreeView
{
Q_OBJECT

public:
    SubaddressView(QWidget* parent = nullptr);

signals:
    void copyAddress();

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif //FEATHER_SUBADDRESSVIEW_H
