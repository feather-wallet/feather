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
{
    ui->setupUi(this);
    this->setWindowTitle("Scan QR Code");

    QPixmap pixmap = QPixmap(":/assets/images/warning.png");
    ui->icon_warning->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    m_cameras = QCameraInfo::availableCameras();
    if (m_cameras.count() < 1) {
        QMessageBox::warning(parent, "QR code scanner", "No available cameras found.");
        this->close();
        return;
    }

    for (const auto &camera : m_cameras) {
#ifdef Q_OS_WIN
        ui->combo_camera->addItem(camera.description());
#else
        ui->combo_camera->addItem(camera.deviceName());
#endif
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
    if (index >= m_cameras.size()) {
        return;
    }

    m_camera.reset(new QCamera(m_cameras.at(index)));

    auto captureMode = QCamera::CaptureStillImage;
    if (m_camera->isCaptureModeSupported(captureMode)) {
        m_camera->setCaptureMode(captureMode);
    }

    connect(m_camera.data(), QOverload<QCamera::Error>::of(&QCamera::error), this, &QrCodeScanDialog::displayCameraError);
    connect(m_camera.data(), &QCamera::statusChanged, [this](QCamera::Status status){
        bool unloaded = (status == QCamera::Status::UnloadedStatus);
        ui->frame_unavailable->setVisible(unloaded);
    });

    m_imageCapture.reset(new QCameraImageCapture(m_camera.data()));
    if (!m_imageCapture->isCaptureDestinationSupported(QCameraImageCapture::CaptureToBuffer)) {
        qDebug()  << "Capture to buffer is NOT supported";
    }

    m_imageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    connect(m_imageCapture.data(), &QCameraImageCapture::imageAvailable, this, &QrCodeScanDialog::processAvailableImage);
    connect(m_imageCapture.data(), QOverload<int, QCameraImageCapture::Error, const QString &>::of(&QCameraImageCapture::error),
            this, &QrCodeScanDialog::displayCaptureError);

    m_camera->setViewfinder(ui->viewfinder);
    m_camera->start();
}

void QrCodeScanDialog::displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, "Image Capture Error", errorString);
}

void QrCodeScanDialog::displayCameraError()
{
    QMessageBox::warning(this, "Camera Error", m_camera->errorString());
}

void QrCodeScanDialog::processAvailableImage(int id, const QVideoFrame &frame) {
    Q_UNUSED(id);
    QImage img = frame.image();
    img.convertTo(QImage::Format_RGB32);
    m_thread->addImage(img);
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