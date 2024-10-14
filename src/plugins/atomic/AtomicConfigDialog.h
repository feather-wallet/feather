// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICCONFIGDIALOG_H
#define FEATHER_ATOMICCONFIGDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QNetworkReply>
#include <QTemporaryFile>

#include <archive.h>
#include "components.h"

namespace Ui {
    class AtomicConfigDialog;
}

class AtomicConfigDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit AtomicConfigDialog(QWidget *parent = nullptr);
    ~AtomicConfigDialog() override;
    static QString getPath();

public slots:
    void extract();

private:
    void downloadBinary();
    int copy_data(struct archive *ar, struct archive *aw);
    void saveSwapPath(QString path);
    QScopedPointer<Ui::AtomicConfigDialog> ui;
    QNetworkReply* archive = nullptr;
    QString tempFile;
    QTemporaryFile* download = nullptr;
};


#endif //FEATHER_ATOMICCONFIGDIALOG_H
