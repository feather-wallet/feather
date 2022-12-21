// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "QrCodeScanDialog.h"
#include "ui_QrCodeScanDialog.h"

#include <QCamera>
#include <QMediaDevices>
#include <QCameraDevice>

QrCodeScanDialog::QrCodeScanDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::QrCodeScanDialog)
{
    ui->setupUi(this);

    m_camera.reset(new QCamera(QMediaDevices::defaultVideoInput()));
    m_captureSession.setCamera(m_camera.data());

    m_captureSession.setVideoOutput(ui->viewfinder);

    m_camera->start();

    const QList<QCameraDevice> availableCameras = QMediaDevices::videoInputs();
}

QrCodeScanDialog::~QrCodeScanDialog()
{
}