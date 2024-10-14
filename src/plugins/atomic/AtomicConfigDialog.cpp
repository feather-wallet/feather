// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicConfigDialog.h"
#include "ui_AtomicConfigDialog.h"

#include <QNetworkReply>
#include <stdio.h>
#include <QFileDialog>
#include "utils/config.h"
#include "utils/Networking.h"

#ifdef Q_OS_WIN
#include <zip.h>
#define OS 1 // "WINDOWS"
#else
#include <archive.h>
#include <archive_entry.h>
#ifdef Q_PROCESSOR_X86_64
#define ARCH 1 // "x86_64"
#elifdef Q_PROCESSOR_ARM_V7
#define ARCH 2 // "armv7")
#else
#define ARCH 3 // "assumes aarch64 or unsupported"
#endif
#ifdef Q_OS_DARWIN
#define OS 2 // "MAC"
#else
#define OS 3 // "LINUX"
#endif
#endif

AtomicConfigDialog::AtomicConfigDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::AtomicConfigDialog)
{
    ui->setupUi(this);

    ui->downloadLabel->setVisible(false);
    connect(ui->btn_autoInstall,&QPushButton::clicked, this, [this] {
        /*
        QMessageBox downloadPopup = QMessageBox(this);
        qDebug() << "opening popup";
        downloadPopup.setText("Downloading swap tool, this usually takes about a minute or two, please wait");
        downloadPopup.setWindowTitle("Swap tool download");
        downloadPopup.show();
        qApp->processEvents();
         */
        AtomicConfigDialog::downloadBinary();
    });
    connect(ui->btn_selectFile,&QPushButton::clicked, this, [this] {
        QString path = QFileDialog::getOpenFileName(this, "Select swap binary file",
                                                    Config::defaultConfigDir().absolutePath(),
                                                    "Binary Executable (*)");
        saveSwapPath(path);
        if(path.isEmpty()){
            return;
        }
        close();
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this]{
        this->accept();
    });

    this->adjustSize();
}

QString AtomicConfigDialog::getPath() {
    QFile* pathFile = new QFile(Config::defaultConfigDir().absolutePath() +"/swapPath.conf");
    pathFile->open(QIODevice::ReadOnly);
    QString toolPath = pathFile->readAll();
    pathFile->close();
    return toolPath;
}

void AtomicConfigDialog::saveSwapPath(QString path) {
    QFile* pathFile = new QFile(Config::defaultConfigDir().absolutePath() +"/swapPath.conf");
    pathFile->open(QIODevice::WriteOnly);
    pathFile->resize(0);
    pathFile->write(path.toStdString().c_str());
    pathFile->close();
}
void AtomicConfigDialog::downloadBinary() {
    auto* network = new Networking(this);
    download = new QTemporaryFile(this);
    download->open();
    tempFile = download->fileName();
    QString url;
    QString swapVersion = "0.13.4";
    QString firstPart = "https://github.com/UnstoppableSwap/core/releases/download/" + swapVersion;
    if(OS == 1) {
        // HARD CODED DOWNload URL CHANGE IF PROBLEMS
        url = QString(firstPart+"/swap_"+ swapVersion + "_Windows_x86_64.zip");
    } else if (OS == 3){
        if (ARCH == 1) {
            url = QString(firstPart+"/swap_"+ swapVersion + "_Linux_x86_64.tar");
        } else if (ARCH == 2) {
            url = QString(firstPart+"/swap_"+ swapVersion + "_Linux_armv7.tar");
        } else {
            qDebug() << "Unsupported architecture";
            throw std::runtime_error("Unsupported architecture");
        }
    } else {
        if (ARCH == 1) {
            url = QString(firstPart + "/swap_"+ swapVersion + "_Darwin_x86_64.tar");
        } else if (ARCH == 3) {
            url = QString(firstPart + "/swap_"+ swapVersion + "_Darwin_aarch64.tar");
        } else {
            qDebug() << "Unsupported architecture";
            throw std::runtime_error("Unsupported architecture");
        }
    }

    archive = network->get(this, url);
    ui->downloadLabel->setVisible(true);
    QStringList answer;
    connect(archive,&QNetworkReply::readyRead, this, [this]{
        QByteArray data= archive->readAll();
        download->write(data.constData(), data.size());
    });
    connect(archive, &QNetworkReply::finished,
            this,&AtomicConfigDialog::extract);
};

void AtomicConfigDialog::extract() {

    ui->downloadLabel->setText("Download Successful, extracting binary to final destination");
    download->close();
    archive->deleteLater();

    auto swapPath = Config::defaultConfigDir().absolutePath();
    swapPath.append("/swap");
    QFile binaryFile(swapPath);
    binaryFile.open(QIODevice::WriteOnly);
    //auto operatingSystem = conf()->get(Config::operatingSystem).toString().toStdString();
    //if(strcmp("WIN",operatingSystem.c_str()) == 0) {
        // UNZIP
#ifdef Q_OS_WIN
        zip *z = zip_open(tempFile.toStdString().c_str(), 0, 0);

        //Search for the file of given name
        const char *name = "swap.exe";
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(z, name, 0, &st);
        int total = st.size;
        int wrote = 0;
        //Alloc memory for its uncompressed contents
        char *contents = new char[BUFSIZ];
        zip_file *f = zip_fopen(z, name, 0);
        while (wrote < total) {
            //Read the compressed file
            zip_fread(f, contents, BUFSIZ);
            binaryFile.write(contents);
            wrote += BUFSIZ;
        }
        zip_fclose(f);
        //And close the archive
        zip_close(z);
        saveSwapPath(swapPath+".exe");
#else
    //} else {

        struct archive *a;
        struct archive *ext;
        struct archive_entry *entry;
        int r;
        std::string savePath;
        a = archive_read_new();
        ext = archive_write_disk_new();
        archive_write_disk_set_options(ext, ARCHIVE_EXTRACT_PERM);

        archive_read_support_format_tar(a);
        r = archive_read_open_filename(a, tempFile.toStdString().c_str(), 10240);
        for (;;) {
            r = archive_read_next_header(a, &entry);
            if (r == ARCHIVE_EOF)
                break;
            savePath = Config::defaultConfigDir().absolutePath().toStdString().append("/").append(archive_entry_pathname(entry));
            archive_entry_set_pathname(entry, savePath.c_str());
            r = archive_write_header(ext, entry);
            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
                }

        archive_read_close(a);
        archive_read_free(a);

        archive_write_close(ext);
        archive_write_free(ext);
        saveSwapPath(swapPath);
#endif
    //}
    qDebug() << "Finished";
    binaryFile.close();
    ui->downloadLabel->setText("Swap tool installation complete, Atomic swaps are ready !");


};
int
AtomicConfigDialog::copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;) {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
            return (ARCHIVE_OK);
        if (r != ARCHIVE_OK)
            return (r);
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK) {
            return (r);
        }
    }
}

AtomicConfigDialog::~AtomicConfigDialog() = default;