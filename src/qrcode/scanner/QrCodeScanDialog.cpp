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
        , m_sink(new QVideoSink(this))
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

    connect(ui->viewfinder->videoSink(), &QVideoSink::videoFrameChanged, this, &QrCodeScanDialog::handleFrameCaptured);

    this->onCameraSwitched(0);

    m_thread = new QrScanThread(this);
    m_thread->start();

    connect(m_thread, &QrScanThread::decoded, this, &QrCodeScanDialog::onDecoded);
}

void QrCodeScanDialog::handleFrameCaptured(const QVideoFrame &frame) {
    QImage img = this->videoFrameToImage(frame);
    m_thread->addImage(img);
}

QImage QrCodeScanDialog::videoFrameToImage(const QVideoFrame &videoFrame)
{
    auto handleType = videoFrame.handleType();

    if (handleType == QVideoFrame::NoHandle) {

        QImage image = videoFrame.toImage();

        if (image.isNull()) {
            return {};
        }

        if (image.format() != QImage::Format_ARGB32) {
            image = image.convertToFormat(QImage::Format_ARGB32);
        }
        
        return image.copy();
    }
    
    return {};
}


void QrCodeScanDialog::onCameraSwitched(int index) {
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (index >= cameras.size()) {
        return;
    }

    m_camera.reset(new QCamera(cameras.at(index)));
    m_captureSession.setCamera(m_camera.data());
    m_captureSession.setVideoOutput(ui->viewfinder);

    connect(m_camera.data(), &QCamera::activeChanged, [this](bool active){
        ui->frame_unavailable->setVisible(!active);
    });

    m_camera->start();
}

void QrCodeScanDialog::onDecoded(const QString &data) {
    decodedString = data;
    this->accept();
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