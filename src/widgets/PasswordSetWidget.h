// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_PASSWORDSETWIDGET_H
#define FEATHER_PASSWORDSETWIDGET_H

#include <QWidget>

namespace Ui {
    class PasswordSetWidget;
}

class PasswordSetWidget : public QWidget
{
Q_OBJECT

public:
    explicit PasswordSetWidget(QWidget *parent = nullptr);
    ~PasswordSetWidget() override;

    QString password();
    bool passwordsMatch();
    void resetFields();

signals:
    void passwordEntryChanged();

private slots:
    void onPasswordEntryChanged();

private:
    QScopedPointer<Ui::PasswordSetWidget> ui;
};


#endif //FEATHER_PASSWORDSETWIDGET_H
