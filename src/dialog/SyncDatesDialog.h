// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SYNCDATESDIALOG_H
#define FEATHER_SYNCDATESDIALOG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
    class SyncDatesDialog;
}

class SyncDatesDialog : public QDialog
{
Q_OBJECT

public:
    explicit SyncDatesDialog(QWidget *parent = nullptr);
    ~SyncDatesDialog() override;

    QDateTime getStartDate() const;
    QDateTime getEndDate() const;

private slots:
    void onAccepted();

private:
    QScopedPointer<Ui::SyncDatesDialog> ui;
    QDateTime m_startDate;
    QDateTime m_endDate;
};

#endif // FEATHER_SYNCDATESDIALOG_H
