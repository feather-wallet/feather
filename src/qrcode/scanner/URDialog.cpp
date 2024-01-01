// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "URDialog.h"
#include "ui_URDialog.h"

#include <QFileDialog>
#include <QInputDialog>

#include "utils/Utils.h"
#include "WalletManager.h"

URDialog::URDialog(QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::URDialog)
{
    ui->setupUi(this);

    connect(ui->btn_loadFile, &QPushButton::clicked, [this]{
        QString fn = QFileDialog::getOpenFileName(this, "Load file", QDir::homePath(), "All Files (*)");
        if (fn.isEmpty()) {
            return;
        }

        QFile file(fn);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QByteArray qdata = file.readAll();
        std::string data = qdata.toStdString();
        file.close();
        
        ui->widgetUR->setData("any", data);
    });
    
    connect(ui->btn_loadClipboard, &QPushButton::clicked, [this]{
        QString qdata = Utils::copyFromClipboard();
        if (qdata.length() < 10) {
            Utils::showError(this, "Not enough data on clipboard to encode as UR");
            return;
        }
        
        std::string data = qdata.toStdString();
        
        ui->widgetUR->setData("any", data);
    });
    
    connect(ui->tabWidget, &QTabWidget::currentChanged, [this](int index){
        if (index == 1) {
            ui->widgetScanner->startCapture(true);
        }
    });
    
    connect(ui->widgetScanner, &QrCodeScanWidget::finished, [this](bool success){
        if (!success) {
           Utils::showError(this, "Unable to scan UR");
           ui->widgetScanner->reset();
           return;
        }

        if (ui->radio_clipboard->isChecked()) {
            Utils::copyToClipboard(QString::fromStdString(ui->widgetScanner->getURData()));
            Utils::showInfo(this, "Data copied to clipboard");
        }
        else if (ui->radio_file->isChecked()) {
            QString fn = QFileDialog::getSaveFileName(this, "Save to file", QDir::homePath(), "ur_data");
            if (fn.isEmpty()) {
                ui->widgetScanner->reset();
                return;
            }

            QFile file{fn};
            if (!file.open(QIODevice::WriteOnly)) {
                Utils::showError(this, "Failed to save file", QString("Could not open file %1 for writing").arg(fn));
                ui->widgetScanner->reset();
                return;
            }

            std::string data = ui->widgetScanner->getURData();
            file.write(data.data(), data.size());
            file.close();

            Utils::showInfo(this, "Successfully saved data to file");
        }

        ui->widgetScanner->reset();
    });

    ui->radio_file->setChecked(true);
    ui->tabWidget->setCurrentIndex(0);
    
    this->resize(600, 700);
}

URDialog::~URDialog() = default;