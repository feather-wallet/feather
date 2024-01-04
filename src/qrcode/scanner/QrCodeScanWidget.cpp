// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "QrCodeScanWidget.h"
#include "ui_QrCodeScanWidget.h"

#include <QPermission>
#include <QMediaDevices>
#include <QComboBox>

#include "utils/config.h"
#include "utils/Icons.h"

QrCodeScanWidget::QrCodeScanWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::QrCodeScanWidget)
        , m_sink(new QVideoSink(this))
        , m_thread(new QrScanThread(this))
{
    ui->setupUi(this);
    
    this->setWindowTitle("Scan QR code");
    
    ui->frame_error->hide();
    ui->frame_error->setInfo(icons()->icon("warning.png"), "Lost connection to camera");
    
    this->refreshCameraList();
    
    connect(ui->combo_camera, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QrCodeScanWidget::onCameraSwitched);
    connect(ui->viewfinder->videoSink(), &QVideoSink::videoFrameChanged, this, &QrCodeScanWidget::handleFrameCaptured);
    connect(ui->btn_refresh, &QPushButton::clicked, [this]{
        this->refreshCameraList();
        this->onCameraSwitched(0);
    });
    connect(m_thread, &QrScanThread::decoded, this, &QrCodeScanWidget::onDecoded);

    connect(ui->check_manualExposure, &QCheckBox::toggled, [this](bool enabled) {
        if (!m_camera) {
            return;
        }

        ui->slider_exposure->setVisible(enabled);
        if (enabled) {
            m_camera->setExposureMode(QCamera::ExposureManual);
        } else {
            // Qt-bug: this does not work for cameras that only support V4L2_EXPOSURE_APERTURE_PRIORITY
            // Check with v4l2-ctl -L
            m_camera->setExposureMode(QCamera::ExposureAuto);
        }
        conf()->set(Config::cameraManualExposure, enabled);
    });

    connect(ui->slider_exposure, &QSlider::valueChanged, [this](int value) {
        if (!m_camera) {
            return;
        }

        float exposure = 0.00033 * value;
        m_camera->setExposureMode(QCamera::ExposureManual);
        m_camera->setManualExposureTime(exposure);
        conf()->set(Config::cameraExposureTime, value);
    });

    ui->check_manualExposure->setVisible(false);
    ui->slider_exposure->setVisible(false);
}

void QrCodeScanWidget::startCapture(bool scan_ur) {
    m_scan_ur = scan_ur;
    ui->progressBar_UR->setVisible(m_scan_ur);
    ui->progressBar_UR->setFormat("Progress: %v%");

    QCameraPermission cameraPermission;
    switch (qApp->checkPermission(cameraPermission)) {
        case Qt::PermissionStatus::Undetermined:
            qDebug() << "Camera permission undetermined";
            qApp->requestPermission(cameraPermission, [this] {
                startCapture(m_scan_ur);
            });
            return;
        case Qt::PermissionStatus::Denied:
            ui->frame_error->setText("No permission to start camera.");
            ui->frame_error->show();
            return;
        case Qt::PermissionStatus::Granted:
            qDebug() << "Camera permission granted";
            break;
    }

    if (ui->combo_camera->count() < 1) {
        ui->frame_error->setText("No cameras found. Attach a camera and press 'Refresh'.");
        ui->frame_error->show();
        return;
    }
    
    this->onCameraSwitched(0);
    
    if (!m_thread->isRunning()) {
        m_thread->start();
    }
}

void QrCodeScanWidget::reset() {
    this->decodedString = "";
    m_done = false;
    ui->progressBar_UR->setValue(0);
    m_decoder = ur::URDecoder();
    m_thread->start();
    m_handleFrames = true;
}

void QrCodeScanWidget::stop() {
    m_camera->stop();
    m_thread->stop();
}

void QrCodeScanWidget::pause() {
    m_handleFrames = false;
}

void QrCodeScanWidget::refreshCameraList() {
    ui->combo_camera->clear();
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    for (const auto &camera : cameras) {
        ui->combo_camera->addItem(camera.description());
    }
}

void QrCodeScanWidget::handleFrameCaptured(const QVideoFrame &frame) {
    if (!m_handleFrames) {
        return;
    }
    
    if (!m_thread->isRunning()) {
        return;
    }

    QImage img = this->videoFrameToImage(frame);
    if (img.format() == QImage::Format_ARGB32) {
        m_thread->addImage(img);
    }
}

QImage QrCodeScanWidget::videoFrameToImage(const QVideoFrame &videoFrame)
{
    QImage image = videoFrame.toImage();

    if (image.isNull()) {
        return {};
    }

    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    return image.copy();
}


void QrCodeScanWidget::onCameraSwitched(int index) {
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();

    if (index < 0) {
        return;
    }
    
    if (index >= cameras.size()) {
        return;
    }

    if (m_camera) {
        m_camera->stop();
    }

    ui->frame_error->setVisible(false);

    m_camera.reset(new QCamera(cameras.at(index), this));
    m_captureSession.setCamera(m_camera.data());
    m_captureSession.setVideoOutput(ui->viewfinder);

    bool manualExposureSupported = m_camera->isExposureModeSupported(QCamera::ExposureManual);
    ui->check_manualExposure->setVisible(manualExposureSupported);

    qDebug() << "Supported camera features: " << m_camera->supportedFeatures();
    qDebug() << "Current focus mode: " << m_camera->focusMode();
    if (m_camera->isExposureModeSupported(QCamera::ExposureBarcode)) {
        qDebug() << "Barcode exposure mode is supported";
    }

    connect(m_camera.data(), &QCamera::activeChanged, [this](bool active){
        ui->frame_error->setText("Lost connection to camera");
        ui->frame_error->setVisible(!active);
    });

    connect(m_camera.data(), &QCamera::errorOccurred, [this](QCamera::Error error, const QString &errorString) {
        if (error == QCamera::Error::CameraError) {
            ui->frame_error->setText(QString("Error: %1").arg(errorString));
            ui->frame_error->setVisible(true);
        }
    });

    m_camera->start();

    bool useManualExposure = conf()->get(Config::cameraManualExposure).toBool() && manualExposureSupported;
    ui->check_manualExposure->setChecked(useManualExposure);
    if (useManualExposure) {
        ui->slider_exposure->setValue(conf()->get(Config::cameraExposureTime).toInt());
    }
}

void QrCodeScanWidget::onDecoded(const QString &data) {
    if (m_done) {
        return;
    }
    
    if (m_scan_ur) {
        bool success = m_decoder.receive_part(data.toStdString());
        if (!success) {
          return;
        }

        ui->progressBar_UR->setValue(m_decoder.estimated_percent_complete() * 100);
        ui->progressBar_UR->setMaximum(100);

        if (m_decoder.is_complete()) {
            m_done = true;
            m_thread->stop();
            emit finished(m_decoder.is_success());
        }

        return;
    }

    decodedString = data;
    m_done = true;
    m_thread->stop();
    emit finished(true);
}

std::string QrCodeScanWidget::getURData() {
    if (!m_decoder.is_success()) {
        return "";
    }

    ur::ByteVector cbor = m_decoder.result_ur().cbor();
    std::string data;
    auto i = cbor.begin();
    auto end = cbor.end();
    ur::CborLite::decodeBytes(i, end, data);
    return data;
}

std::string QrCodeScanWidget::getURType() {
    if (!m_decoder.is_success()) {
        return "";
    }

    return m_decoder.expected_type().value_or("");
}

QString QrCodeScanWidget::getURError() {
    if (!m_decoder.is_failure()) {
        return {};
    }
    return QString::fromStdString(m_decoder.result_error().what());
}

QrCodeScanWidget::~QrCodeScanWidget()
{
    m_thread->stop();
    m_thread->quit();
    if (!m_thread->wait(5000))
    {
        m_thread->terminate();
        m_thread->wait();
    }
}