// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include <QStandardItemModel>
#include <QTableWidget>
#include <QProgressBar>
#include <QMessageBox>
#include <QDesktopServices>

#include "restoreheightwidget.h"
#include "ui_restoreheightwidget.h"
#include "utils/utils.h"

RestoreHeightWidget::RestoreHeightWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RestoreHeightWidget)
{
    ui->setupUi(this);
    ui->lineEdit_restoreHeight->setValidator(new QIntValidator(0, 2147483647, this));

    connect(ui->lineEdit_restoreHeight, &QLineEdit::textEdited, [this](const QString &val){
        if (val.isEmpty()) return;
        this->setHeight(val.toInt());
    });
}

void RestoreHeightWidget::hideSlider(){
    ui->restoreGrid->hide();
}

void RestoreHeightWidget::setHeight(int height) {
    if (height < 0)
        height = 0;

    // Update lineEdit
    ui->lineEdit_restoreHeight->setText(QString::number(height));

    // Update slider
    int date = m_restoreHeightLookup->restoreHeightToDate(height);
    ui->restoreSlider->setValue(date);

    this->updateTimestamp(date);
}

void RestoreHeightWidget::initRestoreHeights(RestoreHeightLookup *lookup) {
    // init slider
    m_restoreHeightLookup = lookup;
    int now = std::time(nullptr);
    QList<int> blockDates = m_restoreHeightLookup->data.keys();
    ui->restoreSlider->setMinimum(blockDates[0]);
    ui->restoreSlider->setMaximum(now);

    connect(ui->restoreSlider, &QSlider::sliderMoved, [this](int date){
        // Update lineEdit
        int blockHeight = m_restoreHeightLookup->dateToRestoreHeight(date);
        ui->lineEdit_restoreHeight->setText(QString::number(blockHeight));

        this->updateTimestamp(date);
    });
}

void RestoreHeightWidget::updateTimestamp(int date) {
    QDateTime timestamp;
    timestamp.setTime_t(date);
    ui->label_restoreHeightDate->setText(timestamp.toString("yyyy-MM-dd"));
}

int RestoreHeightWidget::getHeight() {
    return ui->lineEdit_restoreHeight->text().toInt();
}

RestoreHeightWidget::~RestoreHeightWidget() {
    delete ui;
}
