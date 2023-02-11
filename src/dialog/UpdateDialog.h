// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_UPDATEDIALOG_H
#define FEATHER_UPDATEDIALOG_H

#include <QDialog>
#include <QNetworkReply>
#include <QTimer>

#include "utils/Updater.h"

namespace Ui {
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent, QSharedPointer<Updater> updater);
    ~UpdateDialog() override;

private slots:
    void onDownloadClicked();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(const QString &errMsg);
    void onInstallUpdate();
    void onInstallError(const QString &errMsg);
    void onRestartClicked();
    void onUpdateCheckFailed(const QString &onUpdateCheckFailed);

signals:
    void restartWallet(const QString &binaryFilename);

private:
    void checkForUpdates();
    void noUpdateAvailable();
    void updateAvailable();
    void setStatus(const QString &msg, bool success = false);
    void installUpdateMac();

    QScopedPointer<Ui::UpdateDialog> ui;
    QSharedPointer<Updater> m_updater;

    QString m_downloadUrl;
    QString m_updatePath;

    std::string m_updateZipArchive;

    QTimer m_waitingTimer;

    QNetworkReply *m_reply = nullptr;
};

#endif //FEATHER_UPDATEDIALOG_H
