// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TORINFODIALOG_H
#define FEATHER_TORINFODIALOG_H

#include <QDialog>

#include "appcontext.h"

namespace Ui {
    class TorInfoDialog;
}

class TorInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TorInfoDialog(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~TorInfoDialog() override;

public slots:
    void onLogsUpdated();

private slots:
    void onConnectionStatusChanged(bool connected);
    void onApplySettings();
    void onSettingsChanged();
    void onStopTor();
    void onShowInitSyncConfigDialog();

signals:
    void torSettingsChanged();

private:
    void initConnectionSettings();
    void initPrivacyLevel();

    QScopedPointer<Ui::TorInfoDialog> ui;
    QSharedPointer<AppContext> m_ctx;
};


#endif //FEATHER_TORINFODIALOG_H
