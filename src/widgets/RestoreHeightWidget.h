// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_RESTOREHEIGHTWIDGET_H
#define FEATHER_RESTOREHEIGHTWIDGET_H

#include <QWidget>

namespace Ui {
    class RestoreHeightWidget;
}

class RestoreHeightWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RestoreHeightWidget(QWidget *parent = nullptr);
    ~RestoreHeightWidget() override;

    void setHeight(quint64 restoreHeight);
    int getHeight();

private slots:
    void onCreationDateChanged();
    void onRestoreHeightChanged();

private:
    QScopedPointer<Ui::RestoreHeightWidget> ui;
};


#endif //FEATHER_RESTOREHEIGHTWIDGET_H
