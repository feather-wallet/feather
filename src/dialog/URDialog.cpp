// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "URDialog.h"
#include "ui_URDialog.h"

#include <QFileDialog>

#include "utils/Utils.h"

URDialog::URDialog(QWidget *parent, const QString &data, bool scanOnly)
        : WindowModalDialog(parent)
        , ui(new Ui::URDialog)
{
    ui->setupUi(this);

    if (!data.isEmpty()) {
        ui->btn_loadFile->setVisible(false);
        ui->btn_loadClipboard->setVisible(false);
        ui->tabWidget->setTabVisible(1, false);

        QScreen *currentScreen = QApplication::screenAt(this->geometry().center());
        if (!currentScreen) {
            currentScreen = QApplication::primaryScreen();
        }
        int availableHeight = currentScreen->availableGeometry().height() - 200;
        this->resize(availableHeight, availableHeight);

        std::string d = data.toStdString();
        ui->widgetUR->setData("xmr-viewonly", d);
        return;
    }

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

        if (ui->widgetScanner->getURType() == "xmr-viewonly") {
            QRegularExpression viewOnlyDetails(
                "Secret view key: (?<key>[0-9a-f]{64})\nAddress: (?<address>\\w+)\nRestore height: (?<restoreheight>\\d+)\nWallet name: (?<walletname>\\w+)\n",
                QRegularExpression::CaseInsensitiveOption | QRegularExpression::DotMatchesEverythingOption);
            QString data = QString::fromStdString(ui->widgetScanner->getURData());
            QRegularExpressionMatch match = viewOnlyDetails.match(data);

            if (!match.hasMatch()) {
                Utils::showError(this, "Unable to load view-only details", "Unexpected data");
                return;
            }

            m_viewOnlyDetails.address = match.captured("address");
            m_viewOnlyDetails.key = match.captured("key").toLower();
            m_viewOnlyDetails.restoreHeight = match.captured("restoreheight").toInt();
            m_viewOnlyDetails.walletName = QString("%1_view_only").arg(match.captured("walletname"));

            this->accept();
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

    if (scanOnly) {
        ui->tabWidget->setCurrentIndex(1);
        ui->tabWidget->setTabVisible(0, false);
        ui->radio_clipboard->setVisible(false);
        ui->radio_file->setVisible(false);
        return;
    }

    ui->radio_file->setChecked(true);
    ui->tabWidget->setCurrentIndex(0);
    
    this->resize(600, 700);
}

ViewOnlyDetails URDialog::getViewOnlyDetails() {
    return m_viewOnlyDetails;
}

URDialog::~URDialog() = default;