// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "MultiLineInputDialog.h"
#include "ui_MultiLineInputDialog.h"

#include <QDialog>
#include <QFontMetrics>

#include "utils/Utils.h"

MultiLineInputDialog::MultiLineInputDialog(QWidget *parent, const QString &title, const QString &label, const QStringList &defaultList)
        : WindowModalDialog(parent)
        , ui(new Ui::MultiLineInputDialog)
{
    ui->setupUi(this);

    this->setWindowTitle(title);
    ui->label->setText(label);

    QFontMetrics metrics(ui->plainTextEdit->font());
    int maxWidth = 0;
    for (const QString &line : defaultList) {
        int width = metrics.boundingRect(line).width();
        maxWidth = qMax(maxWidth, width);
    }
    ui->plainTextEdit->setMinimumWidth(maxWidth + 10);

    ui->plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
    ui->plainTextEdit->setPlainText(defaultList.join("\n") + "\n");

    this->adjustSize();
}

QStringList MultiLineInputDialog::getList() {
    return ui->plainTextEdit->toPlainText().split("\n");
}

MultiLineInputDialog::~MultiLineInputDialog() = default;