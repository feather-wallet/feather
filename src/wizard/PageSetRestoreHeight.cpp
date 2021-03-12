// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QValidator>
#include "PageSetRestoreHeight.h"
#include "ui_PageSetRestoreHeight.h"
#include "WalletWizard.h"

PageSetRestoreHeight::PageSetRestoreHeight(AppContext *ctx, WizardFields *fields, QWidget *parent)
        : QWizardPage(parent)
        , ui(new Ui::PageSetRestoreHeight)
        , m_ctx(ctx)
        , m_fields(fields)
{
    ui->setupUi(this);

    QRegExp yearRe(R"(\d{2,4}-\d{1,2}-\d{1,2})");
    QValidator *yearValidator = new QRegExpValidator(yearRe, this);
    ui->line_creationDate->setValidator(yearValidator);

    QRegExp heightRe(R"(\d{7})");
    QValidator *heightValidator = new QRegExpValidator(heightRe, this);
    ui->line_restoreHeight->setValidator(heightValidator);

    QPixmap pixmap = QPixmap(":/assets/images/unpaid.png");
    ui->icon->setPixmap(pixmap.scaledToWidth(32, Qt::SmoothTransformation));

    QPixmap pixmap2 = QPixmap(":/assets/images/info.png");
    ui->warningIcon->setPixmap(pixmap2.scaledToWidth(32, Qt::SmoothTransformation));
    ui->infoIcon->setPixmap(pixmap2.scaledToWidth(32, Qt::SmoothTransformation));

    ui->frame_scanWarning->hide();
    ui->frame_walletAgeWarning->hide();

    connect(ui->line_creationDate, &QLineEdit::textEdited, [this]{
        this->onCreationDateEdited();
        this->completeChanged();
    });
    connect(ui->line_restoreHeight, &QLineEdit::textEdited, [this]{
        this->onRestoreHeightEdited();
        this->completeChanged();
    });
}

void PageSetRestoreHeight::initializePage() {
    this->setTitle("Restore height");
    ui->line_creationDate->setText("");
    ui->line_restoreHeight->setText("");
}

void PageSetRestoreHeight::onCreationDateEdited() {
    auto curDate = QDateTime::currentDateTime().addDays(-7);
    auto date = QDateTime::fromString(ui->line_creationDate->text(), "yyyy-MM-dd");
    if (!date.isValid()) {
        ui->frame_walletAgeWarning->hide();
        ui->frame_scanWarning->hide();
        ui->line_restoreHeight->setText("");
        return;
    }

    QDateTime restoreDate = date > curDate ? curDate : date;
    int timestamp = restoreDate.toSecsSinceEpoch();

    QString restoreHeight = QString::number(m_ctx->restoreHeights[m_ctx->networkType]->dateToRestoreHeight(timestamp));
    ui->line_restoreHeight->setText(restoreHeight);

    this->showScanWarning(restoreDate);
    this->showWalletAgeWarning(restoreDate);
}

void PageSetRestoreHeight::onRestoreHeightEdited() {
    int restoreHeight = ui->line_restoreHeight->text().toInt();
    if (restoreHeight == 0) {
        ui->frame_walletAgeWarning->hide();
        ui->frame_scanWarning->hide();
        ui->line_creationDate->setText("");
        return;
    }

    int timestamp = m_ctx->restoreHeights[m_ctx->networkType]->restoreHeightToDate(restoreHeight);
    auto date = QDateTime::fromSecsSinceEpoch(timestamp);
    ui->line_creationDate->setText(date.toString("yyyy-MM-dd"));

    this->showScanWarning(date);
    this->showWalletAgeWarning(date);
}

void PageSetRestoreHeight::showScanWarning(const QDateTime &date) {
    QString dateString = date.toString("yyyy-MM-dd");
    ui->label_scanWarning->setText(QString("Wallet will not scan for transactions before %1").arg(dateString));
    ui->frame_scanWarning->show();
}

void PageSetRestoreHeight::showWalletAgeWarning(const QDateTime &date) {
    QDateTime yearAgo = QDateTime::currentDateTime().addYears(-1);
    ui->frame_walletAgeWarning->setVisible(date < yearAgo);
}

bool PageSetRestoreHeight::validatePage() {
    m_fields->restoreHeight = ui->line_restoreHeight->text().toInt();
    return true;
}

int PageSetRestoreHeight::nextId() const {
    return WalletWizard::Page_WalletFile;
}

bool PageSetRestoreHeight::isComplete() const {
    return !ui->line_restoreHeight->text().isEmpty();
}
