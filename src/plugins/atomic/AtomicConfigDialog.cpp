// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "AtomicConfigDialog.h"
#include "ui_AtomicConfigDialog.h"

#include <QNetworkReply>
#include <QDir>
#include <stdio.h>
#include <archive.h>
#include <zip.h>

#include <archive_entry.h>
#include <QFileDialog>

#include "utils/config.h"
#include "utils/Networking.h"

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
        Config::instance()->set(Config::swapPath, path);
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

void AtomicConfigDialog::downloadBinary() {
    auto* network = new Networking(this);
    download = new QTemporaryFile(this);
    download->open();
    tempFile = download->fileName();
    QString url;
    auto operatingSystem = Config::instance()->get(Config::operatingSystem).toString().toStdString();
    if(strcmp("WIN",operatingSystem.c_str()) == 0) {
        // HARD CODED DOWNload URL CHANGE IF PROBLEMS
        url = QString("https://github.com/comit-network/xmr-btc-swap/releases/download/0.13.1/swap_0.13.1_Windows_x86_64.zip");
    } else if (strcmp("LINUX",operatingSystem.c_str())==0){
        url = QString("https://github.com/comit-network/xmr-btc-swap/releases/download/0.13.1/swap_0.13.1_Linux_x86_64.tar");
    } else {
        url = QString("https://github.com/comit-network/xmr-btc-swap/releases/download/0.13.1/swap_0.13.1_Linux_x86_64.tar");
    }

    archive = network->get(this, url);
    ui->downloadLabel->setVisible(true);
    QStringList answer;
    connect(archive,&QNetworkReply::readyRead, this, [this]{
        QByteArray data= archive->readAll();
        qDebug() << "received data of size: " << data.size();
        download->write(data.constData(), data.size());
    });
    connect(archive, &QNetworkReply::finished,
            this,&AtomicConfigDialog::extract);
};

void AtomicConfigDialog::extract() {

    ui->downloadLabel->setText("Download Successful, extracting binary to final destination");
    qDebug() << "extracting";
    download->close();
    archive->deleteLater();

    auto swapPath = Config::defaultConfigDir().absolutePath();
    swapPath.append("/swapTool");
    QFile binaryFile(swapPath);
    binaryFile.open(QIODevice::WriteOnly);
    auto operatingSystem = conf()->get(Config::operatingSystem).toString().toStdString();
    if(strcmp("WIN",operatingSystem.c_str()) == 0) {
        // UNZIP
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
        conf()->set(Config::swapPath,swapPath);
    } else {

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
            qDebug() << savePath;
            archive_entry_set_pathname(entry, savePath.c_str());
            r = archive_write_header(ext, entry);
            copy_data(a, ext);
            r = archive_write_finish_entry(ext);
                }

        archive_read_close(a);
        archive_read_free(a);

        archive_write_close(ext);
        archive_write_free(ext);
        conf()->set(Config::swapPath, QString(savePath.c_str()));
    }
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