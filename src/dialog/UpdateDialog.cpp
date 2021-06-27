// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"

#include <QFileDialog>
#include <utility>

#include "utils/networking.h"
#include "utils/NetworkManager.h"
#include "utils/AsyncTask.h"
#include "utils/Updater.h"
#include "utils/utils.h"

#include "zip.h"

UpdateDialog::UpdateDialog(QWidget *parent, QString version, QString downloadUrl, QString hash, QString signer)
        : QDialog(parent)
        , ui(new Ui::UpdateDialog)
        , m_version(std::move(version))
        , m_downloadUrl(std::move(downloadUrl))
        , m_hash(std::move(hash))
        , m_signer(std::move(signer))
{
    ui->setupUi(this);

    ui->btn_installUpdate->hide();
    ui->btn_restart->hide();
    ui->progressBar->hide();

    auto bigFont = Utils::relativeFont(4);
    ui->label_header->setFont(bigFont);
    ui->label_header->setText(QString("New Feather version %1 is available").arg(m_version));

    connect(ui->btn_cancel, &QPushButton::clicked, [this]{
        if (m_reply) {
            m_reply->abort();
        }
        this->reject();
    });
    connect(ui->btn_download, &QPushButton::clicked, this, &UpdateDialog::onDownloadClicked);
    connect(ui->btn_installUpdate, &QPushButton::clicked, this, &UpdateDialog::onInstallUpdate);
    connect(ui->btn_restart, &QPushButton::clicked, this, &UpdateDialog::onRestartClicked);

    this->adjustSize();
}

void UpdateDialog::onDownloadClicked() {
    ui->label_body->setText("Downloading update..");
    ui->btn_download->hide();
    ui->progressBar->show();

    UtilsNetworking network{getNetworkTor()};

    m_reply = network.get(m_downloadUrl);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &UpdateDialog::onDownloadProgress);
    connect(m_reply, &QNetworkReply::finished, this, &UpdateDialog::onDownloadFinished);
}

void UpdateDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    ui->progressBar->setValue(bytesReceived);
    ui->progressBar->setMaximum(bytesTotal);
}

void UpdateDialog::onDownloadFinished() {
    bool error = (m_reply->error() != QNetworkReply::NoError);
    if (error) {
        this->onDownloadError(QString("Network error: %1").arg(m_reply->errorString()));
        return;
    }

    QByteArray response = m_reply->readAll();
    if (response.isEmpty()) {
        this->onDownloadError("Network error: Empty response");
        return;
    }

    std::string responseStr = response.toStdString();

    try {
        const QByteArray calculatedHash = AsyncTask::runAndWaitForFuture([this, responseStr]{
            return Updater().getHash(&responseStr[0], responseStr.size());
        });

        const QByteArray signedHash = QByteArray::fromHex(m_hash.toUtf8());

        if (signedHash != calculatedHash) {
            this->onDownloadError("Error: Hash sum mismatch.");
            return;
        }
    }
    catch (const std::exception &e)
    {
        this->onDownloadError(QString("Error: Unable to calculate sha256sum: %1").arg(e.what()));
        return;
    }

    this->setStatus("Download finished and verified.", true);

    ui->btn_installUpdate->show();
    ui->progressBar->hide();

    m_updateZipArchive = responseStr;
}

void UpdateDialog::onDownloadError(const QString &errMsg) {
    // Clean up so download can be retried
    this->setStatus(errMsg);
    ui->progressBar->hide();
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->btn_download->show();
    ui->btn_download->setText("Retry download");
}

void UpdateDialog::onInstallUpdate() {
    ui->btn_installUpdate->hide();
    this->setStatus("Unzipping archive...");

    zip_error_t err;
    zip_error_init(&err);

    zip_source_t *zip_source = zip_source_buffer_create(&m_updateZipArchive[0], m_updateZipArchive.size(), 0, &err);
    if (!zip_source) {
        this->onInstallError(QString("Error in libzip: Unable to create zip source from buffer: %1").arg(QString::fromStdString(err.str)));
        return;
    }

    zip_t *zip_archive = zip_open_from_source(zip_source, 0, &err);
    if (!zip_archive) {
        this->onInstallError(QString("Error in libzip: Unable to open archive from source: %1").arg(QString::fromStdString(err.str)));
        return;
    }

    auto num_entries = zip_get_num_entries(zip_archive, 0);
    if (num_entries <= 0) {
        this->onInstallError("Error in libzip: Archive has no entries");
        return;
    }

    // We only expect the archive to contain 1 file
    std::string fname = zip_get_name(zip_archive, 0, 0);
    if (fname.empty()) {
        this->onInstallError("Error in libzip: Invalid filename in archive");
        return;
    }

    struct zip_stat sb;
    if (zip_stat_index(zip_archive, 0, 0, &sb) != 0) {
        this->onInstallError("Error in libzip: Entry index not found");
        return;
    }

    QString name = QString::fromStdString(sb.name);
    qDebug() << "File found in archive: " << name << ", with size: " << QString::number(sb.size);

    struct zip_file *zf;
    zf = zip_fopen_index(zip_archive, 0, 0);
    if (!zf) {
        this->onInstallError("Error in libzip: Unable to open entry");
        return;
    }

    std::unique_ptr<char[]> contents{new char[sb.size]};

    auto bytes_read = zip_fread(zf, contents.get(), sb.size);
    if (bytes_read != sb.size){
        this->onInstallError("Error in libzip: File size inconsistent");
        return;
    }

    zip_fclose(zf);
    zip_close(zip_archive);

    QString applicationPath = qgetenv("APPIMAGE");
    if (!applicationPath.isEmpty()) {
        applicationPath = QFileInfo(applicationPath).absoluteDir().path();
    } else {
        applicationPath = QCoreApplication::applicationDirPath();
    }

    QDir applicationDir(applicationPath);
    QString filePath = applicationDir.filePath(name);
    m_updatePath = filePath;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        this->onInstallError(QString("Error: Could not write to application path: %1").arg(filePath));
        return;
    }

    if (static_cast<size_t>(file.write(&contents[0], sb.size)) != sb.size) {
        this->onInstallError("Error: Unable to write file");
        return;
    }

    if (!file.setPermissions(QFile::ExeUser | QFile::ExeOwner | QFile::ExeGroup | QFile::ExeOther
                             | QFile::ReadUser | QFile::ReadOwner
                             | QFile::WriteUser | QFile::WriteOwner)) {
        this->onInstallError("Error: Unable to set executable flags");
        return;
    }

    this->setStatus("Installation successful. Do you want to restart Feather now?");
    ui->btn_restart->show();
}

void UpdateDialog::onInstallError(const QString &errMsg) {
    this->setStatus(errMsg);
}

void UpdateDialog::onRestartClicked() {
    emit restartWallet(m_updatePath);
}

void UpdateDialog::setStatus(const QString &msg, bool success) {
    ui->label_body->setText(msg);
    if (success)
        ui->label_body->setStyleSheet("QLabel { color : #2EB358; }");
    else
        ui->label_body->setStyleSheet("");
}

UpdateDialog::~UpdateDialog() = default;