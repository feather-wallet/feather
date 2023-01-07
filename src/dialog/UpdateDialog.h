// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_UPDATEDIALOG_H
#define FEATHER_UPDATEDIALOG_H

#include <QDialog>
#include <QNetworkReply>

namespace Ui {
    class UpdateDialog;
}

class UpdateDialog : public QDialog
{
Q_OBJECT

public:
    explicit UpdateDialog(QWidget *parent, QString version, QString downloadUrl, QString hash, QString signer, QString platformTag);
    ~UpdateDialog() override;

private slots:
    void onDownloadClicked();
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(const QString &errMsg);
    void onInstallUpdate();
    void onInstallError(const QString &errMsg);
    void onRestartClicked();

signals:
    void restartWallet(const QString &binaryFilename);

private:
    void setStatus(const QString &msg, bool success = false);
    void installUpdateMac();

    QScopedPointer<Ui::UpdateDialog> ui;

    QString m_version;
    QString m_downloadUrl;
    QString m_hash;
    QString m_signer;
    QString m_platformTag;

    QString m_updatePath;

    std::string m_updateZipArchive;

    QNetworkReply *m_reply = nullptr;
};

#endif //FEATHER_UPDATEDIALOG_H
