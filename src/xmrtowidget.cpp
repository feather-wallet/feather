// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "xmrtowidget.h"
#include "ui_xmrtowidget.h"
#include "dialog/xmrtoinfodialog.h"
#include "libwalletqt/WalletManager.h"
#include "mainwindow.h"
#include "globals.h"

#include <QMenu>
#include <QMessageBox>

XMRToWidget::XMRToWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::XMRToWidget)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    QString amount_rx = R"(^\d*\.\d*$)";
    QRegExp rx;
    rx.setPattern(amount_rx);
    QValidator *validator =  new QRegExpValidator(rx, this);
    ui->lineAmount->setValidator(validator);

    // xmrto logo (c) binaryFate et. al. :-D
    QPixmap p(":assets/images/xmrto_big.png");
    ui->logo->setPixmap(p.scaled(112, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->ratesLayout->hide();

    // context menu
    m_contextMenu = new QMenu();
    m_showDetailsAction = m_contextMenu->addAction("Details");
    m_viewOnXmrToAction = m_contextMenu->addAction("View order on XMR.to");
    m_viewOnXmrToAction->setIcon(QIcon(":/assets/images/xmrto.png"));
    connect(m_showDetailsAction, &QAction::triggered, this, &XMRToWidget::showInfoDialog);
    connect(m_viewOnXmrToAction, &QAction::triggered, [&](){
        QModelIndex index = ui->historyTable->currentIndex();
        XmrToOrder *order = this->tableModel->orders->at(index.row());
        emit viewOrder(order->uuid);
    });

    // connects
    connect(ui->btnGetRates, &QPushButton::pressed, this, &XMRToWidget::onGetRates);
    connect(ui->lineAmount, &QLineEdit::textChanged, this, &XMRToWidget::updateConversionLabel);
    connect(ui->comboBox_currency, &QComboBox::currentTextChanged, this, &XMRToWidget::updateConversionLabel);
    connect(ui->torCheckBox, &QCheckBox::stateChanged, this, &XMRToWidget::onTorCheckBoxToggled);
    connect(ui->btnCreate, &QPushButton::clicked, this, &XMRToWidget::onCreateOrder);

    ui->historyTable->header()->setStretchLastSection(true);
    ui->historyTable->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->historyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->historyTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->historyTable, &QTreeView::customContextMenuRequested, [&](const QPoint & point){
        QModelIndex index = ui->historyTable->indexAt(point);
        if (index.isValid()) {
            m_contextMenu->popup(ui->historyTable->viewport()->mapToGlobal(point));
        }
    });

    if (m_ctx->isTails || m_ctx->isWhonix) {
        ui->torCheckBox->setDisabled(true);
    }

    connect(ui->historyTable, &QTreeView::doubleClicked, this, &XMRToWidget::showInfoDialog);
}

void XMRToWidget::setHistoryModel(XmrToModel *model) {
    this->tableModel = model;
    this->ui->historyTable->setModel(model);
}

void XMRToWidget::onBalanceUpdated(quint64 balance, quint64 spendable) {
    this->m_unlockedBalance = spendable / globals::cdiv;
}

void XMRToWidget::onWalletClosed() {
    ui->lineAddress->clear();
    ui->lineAmount->clear();
    ui->xmrLabelEstimate->setText("0.00 XMR");
}

void XMRToWidget::onCreateOrder() {
    // @TODO: regex verify

    auto amount = ui->lineAmount->text();
    if(amount.isEmpty()) {
        QMessageBox::warning(this, "Cannot create XMR.To order", "Invalid amount");
        return;
    }

    double amount_num = amount.toDouble();
    QString amount_cur = (ui->comboBox_currency->currentIndex() == curr::BTC) ? "BTC" : "XMR";
    double amount_xmr = amount_num;
    if (ui->comboBox_currency->currentIndex() == curr::BTC) {
        amount_xmr = AppContext::prices->convert("BTC", "XMR", amount_num);
    }

    auto available = m_unlockedBalance;
    if(amount_xmr > available){
        QMessageBox::warning(this, "Cannot create XMR.To order", "Not enough Monero to create order.");
        return;
    }

    ui->btnGetRates->setEnabled(false);
    ui->btnCreate->setEnabled(false);

    auto btc_address = ui->lineAddress->text();
    emit createOrder(amount_num, amount_cur, btc_address);

    QTimer::singleShot(2000, [=] {
        ui->lineAmount->clear();
        ui->lineAddress->clear();
        ui->btnGetRates->setEnabled(true);
    });
}

void XMRToWidget::onTorCheckBoxToggled(int state) {
    ui->btnGetRates->setEnabled(true);
    ui->btnCreate->setEnabled(false);
    emit networkChanged(!state);
}

void XMRToWidget::updateConversionLabel() {
    QString amount = ui->lineAmount->text();

    int curIndex = ui->comboBox_currency->currentIndex();
    QString symbolFrom = (curIndex == curr::XMR) ? "XMR" : "BTC";
    QString symbolTo = (curIndex == curr::XMR) ? "BTC" : "XMR";

    if(amount.isEmpty()) {
        ui->xmrLabelEstimate->setText(QString("0.00 %1").arg(symbolTo));
        return;
    }
    auto amount_num = amount.toDouble();
    auto amount_converted = AppContext::prices->convert(symbolFrom, symbolTo, amount_num);
    auto amount_converted_str = QString::number(amount_converted, 'f', 2);

    auto fiat_cur = config()->get(Config::preferredFiatCurrency).toString();
    auto amount_fiat = AppContext::prices->convert(symbolFrom, fiat_cur, amount_num);
    auto amount_fiat_str = QString::number(amount_fiat, 'f', 2);

    ui->xmrLabelEstimate->setText(QString("%1 %2, %3 %4").arg(amount_converted_str, symbolTo, amount_fiat_str, fiat_cur));
}

void XMRToWidget::onGetRates() {
    ui->btnGetRates->setEnabled(false);
    ui->btnCreate->setEnabled(false);
    emit getRates();
}

void XMRToWidget::onConnectionError(QString msg) {
    ui->btnGetRates->setEnabled(true);
    ui->btnCreate->setEnabled(false);
    msg = QString("%1\n\n%2").arg(msg).arg(m_regionBlockMessage);
    QMessageBox::warning(this, "XMR.To Connection Error", msg);
}

void XMRToWidget::onConnectionSuccess() {
    ui->btnGetRates->setEnabled(true);
    ui->btnCreate->setEnabled(true);
}

void XMRToWidget::onRatesUpdated(XmrToRates rates) {
    ui->label_rate->setText(QString("%1 BTC").arg(QString::number(rates.price)));
    ui->label_minimum->setText(QString("%1 BTC").arg(QString::number(rates.lower_limit)));
    ui->label_maximum->setText(QString("%1 BTC").arg(QString::number(rates.upper_limit)));

    if(!m_ratesDisplayed) {
        ui->ratesLayout->setVisible(true);
        m_ratesDisplayed = true;
    }
}

void XMRToWidget::onInitiateTransaction() {
    ui->btnCreate->setEnabled(false);
}

void XMRToWidget::onEndTransaction() {
    ui->btnCreate->setEnabled(true);
}

void XMRToWidget::showInfoDialog() {
    QModelIndex index = ui->historyTable->currentIndex();
    XmrToOrder *order = this->tableModel->orders->at(index.row());
    auto *dialog = new XmrToInfoDialog(order, this);
    dialog->exec();
    dialog->deleteLater();
}

XMRToWidget::~XMRToWidget() {
    delete ui;
}
