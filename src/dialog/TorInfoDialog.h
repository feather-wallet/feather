// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_TORINFODIALOG_H
#define FEATHER_TORINFODIALOG_H

#include <QDialog>

namespace Ui {
    class TorInfoDialog;
}

class TorInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TorInfoDialog(QWidget *parent = nullptr);
    ~TorInfoDialog() override;

public slots:
    void onLogsUpdated();

private slots:
    void onConnectionStatusChanged(bool connected);
    void onStatusChanged(const QString &msg = "");

private:
    QScopedPointer<Ui::TorInfoDialog> ui;
};


#endif //FEATHER_TORINFODIALOG_H
