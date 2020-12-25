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
    connect(ui->lineEdit_restoreHeight, &QLineEdit::textEdited, [=](const QString &val){
        // update slider on lineEdit change
        if(val.isEmpty()) return;
        auto height = val.toUInt();
        if(height <= 1) return;
        auto timestamp = m_restoreHeightLookup->restoreHeightToDate(height);
        ui->restoreSlider->blockSignals(true);
        ui->restoreSlider->setValue(timestamp);
        ui->restoreSlider->blockSignals(false);
    });
}

void RestoreHeightWidget::hideSlider(){
    ui->restoreGrid->hide();
}

void RestoreHeightWidget::initRestoreHeights(RestoreHeightLookup *lookup) {
    // init slider
    m_restoreHeightLookup = lookup;
    int now = std::time(nullptr);
    QList<int> blockDates = m_restoreHeightLookup->data.keys();
    ui->restoreSlider->setMinimum(blockDates[0]);
    ui->restoreSlider->setMaximum(now);
    connect(ui->restoreSlider, &QSlider::valueChanged, this, &RestoreHeightWidget::onValueChanged);
}

void RestoreHeightWidget::onValueChanged(int date) {
    QDateTime timestamp;
    timestamp.setTime_t(date);
    ui->label_restoreHeightDate->setText(timestamp.toString("yyyy-MM-dd"));
    auto blockHeight = m_restoreHeightLookup->dateToRestoreHeight(date);
    ui->lineEdit_restoreHeight->setText(QString::number(blockHeight));
}

int RestoreHeightWidget::getHeight() {
    return ui->lineEdit_restoreHeight->text().toInt();
}

RestoreHeightWidget::~RestoreHeightWidget() {
    delete ui;
}
