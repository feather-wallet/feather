// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "QrCodeScanDialog.h"
#include "ui_QrCodeScanDialog.h"

#include <QCamera>
#include <QMediaDevices>
#include <QCameraDevice>
#include <QMessageBox>
#include <QImageCapture>
#include <QVideoFrame>

#include "Utils.h"

QrCodeScanDialog::QrCodeScanDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::QrCodeScanDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("Scan QR code");

    QPixmap pixmap = QPixmap(":/assets/images/warning.png");
    ui->icon_warning->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const auto &camera : cameras) {
        ui->combo_camera->addItem(camera.description());
    }

    connect(ui->combo_camera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QrCodeScanDialog::onCameraSwitched);

    this->onCameraSwitched(0);

    m_thread = new QrScanThread(this);
    m_thread->start();

    connect(m_thread, &QrScanThread::decoded, this, &QrCodeScanDialog::onDecoded);
    connect(m_thread, &QrScanThread::notifyError, this, &QrCodeScanDialog::notifyError);

    connect(&m_imageTimer, &QTimer::timeout, this, &QrCodeScanDialog::takeImage);
    m_imageTimer.start(500);
}

void QrCodeScanDialog::onCameraSwitched(int index) {
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (index >= cameras.size()) {
        return;
    }

    m_camera.reset(new QCamera(cameras.at(index)));
    m_captureSession.setCamera(m_camera.data());
    m_captureSession.setVideoOutput(ui->viewfinder);

    m_imageCapture = new QImageCapture;
    m_captureSession.setImageCapture(m_imageCapture);

    connect(m_imageCapture, &QImageCapture::imageCaptured, this, &QrCodeScanDialog::processCapturedImage);
    connect(m_camera.data(), &QCamera::errorOccurred, this, &QrCodeScanDialog::displayCameraError);
    connect(m_camera.data(), &QCamera::activeChanged, [this](bool active){
        ui->frame_unavailable->setVisible(!active);
    });

    m_camera->start();
}

void QrCodeScanDialog::processCapturedImage(int requestId, const QImage& img) {
    Q_UNUSED(requestId);
    QImage image{img};
    image.convertTo(QImage::Format_RGB32);
    m_thread->addImage(image);
}

void QrCodeScanDialog::takeImage()
{
    if (m_imageCapture->isReadyForCapture()) {
        m_imageCapture->capture();
    }
}

void QrCodeScanDialog::onDecoded(int type, const QString &data) {
    decodedString = data;
    this->accept();
}

void QrCodeScanDialog::displayCameraError()
{
    if (m_camera->error() != QCamera::NoError) {
        Utils::showError(this, "Camera error", m_camera->errorString());
    }
}

void QrCodeScanDialog::notifyError(const QString &msg) {
    qDebug() << "QrScanner error: " << msg;
}

QrCodeScanDialog::~QrCodeScanDialog()
{
    m_thread->stop();
    m_thread->quit();
    if (!m_thread->wait(5000))
    {
        m_thread->terminate();
        m_thread->wait();
    }
}