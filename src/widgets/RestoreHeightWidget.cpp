// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "RestoreHeightWidget.h"
#include "ui_RestoreHeightWidget.h"

#include <QValidator>

#include "AppData.h"
#include "constants.h"

RestoreHeightWidget::RestoreHeightWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::RestoreHeightWidget)
{
    ui->setupUi(this);

    QRegExp yearRe(R"(\d{2,4}-\d{1,2}-\d{1,2})");
    QValidator *yearValidator = new QRegExpValidator(yearRe, this);
    ui->line_creationDate->setValidator(yearValidator);

    QRegExp heightRe(R"(\d{7})");
    QValidator *heightValidator = new QRegExpValidator(heightRe, this);
    ui->line_restoreHeight->setValidator(heightValidator);

    connect(ui->line_creationDate, &QLineEdit::textEdited, this, &RestoreHeightWidget::onCreationDateChanged);
    connect(ui->line_restoreHeight, &QLineEdit::textEdited, this, &RestoreHeightWidget::onRestoreHeightChanged);
}

void RestoreHeightWidget::setHeight(quint64 restoreHeight) {
    ui->line_restoreHeight->setText(QString::number(restoreHeight));
    this->onRestoreHeightChanged();
}

int RestoreHeightWidget::getHeight() {
    return ui->line_restoreHeight->text().toInt();
}

void RestoreHeightWidget::onCreationDateChanged() {
    auto curDate = QDateTime::currentDateTime().addDays(-7);
    auto date = QDateTime::fromString(ui->line_creationDate->text(), "yyyy-MM-dd");
    if (!date.isValid()) {
        return;
    }

    QDateTime restoreDate = date > curDate ? curDate : date;
    qint64 timestamp = restoreDate.toSecsSinceEpoch();

    QString restoreHeight = QString::number(appData()->restoreHeights[constants::networkType]->dateToHeight(timestamp));
    ui->line_restoreHeight->setText(restoreHeight);
}

void RestoreHeightWidget::onRestoreHeightChanged() {
    int restoreHeight = ui->line_restoreHeight->text().toInt();
    QDateTime date = appData()->restoreHeights[constants::networkType]->heightToDate(restoreHeight);
    ui->line_creationDate->setText(date.toString("yyyy-MM-dd"));
}

RestoreHeightWidget::~RestoreHeightWidget() = default;