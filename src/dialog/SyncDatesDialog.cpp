// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "SyncDatesDialog.h"
#include "ui_SyncDatesDialog.h"

#include <QMessageBox>

SyncDatesDialog::SyncDatesDialog(QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::SyncDatesDialog)
{
    ui->setupUi(this);

    // Set default dates
    QDateTime now = QDateTime::currentDateTime();
    ui->dateEdit_start->setDateTime(now.addMonths(-1)); // Default to 1 month ago
    ui->dateEdit_end->setDateTime(now);

    // Set reasonable date ranges
    QDateTime genesisTime = QDateTime::fromSecsSinceEpoch(1397818193, Qt::UTC); // Monero genesis
    ui->dateEdit_start->setMinimumDateTime(genesisTime);
    ui->dateEdit_start->setMaximumDateTime(now);
    ui->dateEdit_end->setMinimumDateTime(genesisTime);
    ui->dateEdit_end->setMaximumDateTime(now);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SyncDatesDialog::onAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    this->adjustSize();
}

QDateTime SyncDatesDialog::getStartDate() const {
    return m_startDate;
}

QDateTime SyncDatesDialog::getEndDate() const {
    return m_endDate;
}

void SyncDatesDialog::onAccepted() {
    m_startDate = ui->dateEdit_start->dateTime();
    m_endDate = ui->dateEdit_end->dateTime();

    if (m_startDate >= m_endDate) {
        QMessageBox::warning(this, "Invalid Date Range", "Start date must be before end date.");
        return;
    }

    this->accept();
}

SyncDatesDialog::~SyncDatesDialog() = default;
