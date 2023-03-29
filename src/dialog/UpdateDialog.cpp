// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "UpdateDialog.h"
#include "ui_UpdateDialog.h"

#include <QFileDialog>

#include "constants.h"
#include "utils/AsyncTask.h"
#include "utils/Networking.h"
#include "utils/NetworkManager.h"
#include "utils/Updater.h"
#include "utils/Utils.h"
#include "utils/SemanticVersion.h"

#include "zip.h"

UpdateDialog::UpdateDialog(QWidget *parent, QSharedPointer<Updater> updater)
    : QDialog(parent)
    , ui(new Ui::UpdateDialog)
    , m_updater(std::move(updater))
{
    ui->setupUi(this);

    auto bigFont = Utils::relativeFont(4);
    ui->label_header->setFont(bigFont);
    ui->frame->hide();

    connect(m_updater.data(), &Updater::updateAvailable, this, &UpdateDialog::updateAvailable);
    connect(m_updater.data(), &Updater::noUpdateAvailable, this, &UpdateDialog::noUpdateAvailable);
    connect(m_updater.data(), &Updater::updateCheckFailed, this, &UpdateDialog::onUpdateCheckFailed);

    bool updateAvailable = (m_updater->state == Updater::State::UPDATE_AVAILABLE);
    if (updateAvailable) {
        this->updateAvailable();
    } else {
        this->checkForUpdates();
    }

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

void UpdateDialog::checkForUpdates() {
    ui->label_header->setText("Checking for updates");
    ui->label_body->setText("..");
    connect(&m_waitingTimer, &QTimer::timeout, [this]{
       ui->label_body->setText(ui->label_body->text() + ".");
    });
    m_waitingTimer.start(500);
    m_updater->checkForUpdates();
}

void UpdateDialog::noUpdateAvailable() {
    m_waitingTimer.stop();
    this->setStatus("Feather is up-to-date.", true);
}

void UpdateDialog::updateAvailable() {
    m_waitingTimer.stop();
    ui->frame->show();
    ui->btn_installUpdate->hide();
    ui->btn_restart->hide();
    ui->progressBar->hide();
    ui->label_header->setText(QString("New Feather version %1 is available").arg(m_updater->version));
    ui->label_body->setText("Do you want to download and verify the new version?");
}

void UpdateDialog::onUpdateCheckFailed(const QString &errorMsg) {
    m_waitingTimer.stop();
    this->setStatus(QString("Failed to check for updates: %1").arg(errorMsg), false);
}

void UpdateDialog::onDownloadClicked() {
    ui->label_body->setText("Downloading update..");
    ui->btn_download->hide();
    ui->progressBar->show();

    Networking network{this};

    m_reply = network.get(m_updater->downloadUrl);
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

        const QByteArray signedHash = QByteArray::fromHex(m_updater->hash.toUtf8());

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

#ifdef Q_OS_MACOS
    this->installUpdateMac();
    return;
#endif

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

    QDir applicationDir(Utils::applicationPath());
    QString filePath = applicationDir.filePath(name);
    if (m_updater->platformTag == "win-installer") {
        filePath = QString("%1/%2").arg(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation), name);
    }

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

    if (m_updater->platformTag == "win-installer") {
        this->setStatus("Installer written. Click 'restart' to close Feather and start the installer.");
    } else {
        this->setStatus("Installation successful. Do you want to restart Feather now?");
    }
    ui->btn_restart->show();
}

void UpdateDialog::installUpdateMac() {
    QString appPath = Utils::applicationPath();
    QDir appDir(appPath);
    if (appPath.endsWith("Contents/MacOS")) {
        appDir.cd("../..");
    }

    if (appPath.contains("AppTranslocation")) {
        this->onInstallError("Error: Application is translocated. Please move it to the Applications folder and try again.");
        return;
    }

    if (!SemanticVersion::isValid(SemanticVersion::fromString(m_updater->version))) {
        this->onInstallError(QString("Error: Invalid version: %1").arg(m_updater->version));
        return;
    }

    QString appName = QString("feather-%1").arg(m_updater->version);
    QString zipName = QString("%1.zip").arg(appName);

    QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (downloadsPath.isEmpty()) {
        this->onInstallError(QString("Error: Could not determine download location"));
        return;
    }

    QString fPath = QString("%1/%2").arg(downloadsPath, zipName);
    QFile file(fPath);
    qDebug() << "Writing zip file to " << fPath;
    if (!file.open(QIODevice::WriteOnly)) {
        this->onInstallError(QString("Error: Could not write to download location: %1").arg(fPath));
        return;
    }

    if (static_cast<size_t>(file.write(&m_updateZipArchive[0], m_updateZipArchive.size())) != m_updateZipArchive.size()) {
        this->onInstallError("Error: Unable to write file");
        return;
    }

    QProcess unzip;
    unzip.start("/usr/bin/unzip", {"-o", fPath, "-d", appDir.absolutePath()});
    unzip.waitForFinished();

    if (unzip.exitStatus() != QProcess::NormalExit) {
        this->onInstallError(QString("Error: Unable to extract: %1").arg(fPath));
        return;
    }

    m_updatePath = QString("%1/Contents/MacOS/feather").arg(appDir.absolutePath());
    qDebug() << "Update path: " << m_updatePath;

    file.remove();

    this->setStatus(QString("Installation successful: Do you want to restart Feather now?").arg(m_updatePath));
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