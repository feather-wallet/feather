// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "QrCodeScanDialog.h"
#include "ui_QrCodeScanDialog.h"

#include <QMessageBox>
#include <QtMultimedia/QCamera>
#include <QtMultimedia/QCameraInfo>

QrCodeScanDialog::QrCodeScanDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::QrCodeScanDialog)
    , m_scanner(new QrCodeScanner(this))
{
    ui->setupUi(this);
    this->setWindowTitle("Scan QR Code");

    QPixmap pixmap = QPixmap(":/assets/images/warning.png");
    ui->icon_warning->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    m_cameras = QCameraInfo::availableCameras();
    if (m_cameras.count() < 1) {
        QMessageBox::warning(parent, "QR code scanner", "No available cameras found.");
        this->close();
    }

    for (const auto &camera : m_cameras) {
        ui->combo_camera->addItem(camera.deviceName());
    }

    connect(ui->combo_camera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QrCodeScanDialog::onCameraSwitched);

    connect(m_scanner, &QrCodeScanner::decoded, this, &QrCodeScanDialog::onDecoded);
    connect(m_scanner, &QrCodeScanner::notifyError, this, &QrCodeScanDialog::notifyError);

    this->onCameraSwitched(0);
}

void QrCodeScanDialog::onCameraSwitched(int index) {
    if (index >= m_cameras.size()) {
        return;
    }

    m_scanner->setSource(nullptr);
    delete m_camera;

    m_camera = new QCamera(m_cameras.at(index), this);
    connect(m_camera, &QCamera::statusChanged, [this](QCamera::Status status){
        bool unloaded = (status == QCamera::Status::UnloadedStatus);
        ui->frame_unavailable->setVisible(unloaded);
    });

    m_camera->setCaptureMode(QCamera::CaptureViewfinder);
    m_camera->setViewfinder(ui->viewfinder);

    m_scanner->setSource(m_camera);
    m_scanner->setEnabled(true);

    m_camera->start();
}

void QrCodeScanDialog::onDecoded(int type, const QString &data) {
    decodedString = data;
    this->accept();
}

void QrCodeScanDialog::notifyError(const QString &msg) {
    qDebug() << "QrScanner error: " << msg;
}

QrCodeScanDialog::~QrCodeScanDialog()
{
    delete m_camera;
    delete ui;
}