// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TrocadorAppWidget.h"
#include "ui_TrocadorAppWidget.h"

#include <QMessageBox>
#include <QMenu>

#include "TrocadorAppInfoDialog.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/WebsocketNotifier.h"

TrocadorAppWidget::TrocadorAppWidget(QWidget *parent, Wallet *wallet)
        : QWidget(parent)
        , ui(new Ui::TrocadorAppWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    QPixmap logo(":/assets/images/trocadorApp_logo.png");
    ui->logo->setPixmap(logo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    ui->combo_currency->addItems(config()->get(Config::trocadorAppCryptoSymbols).toStringList());
    ui->combo_currency->setCurrentText(config()->get(Config::trocadorAppCryptoSymbols).toStringList().filter("XMR").first());
    ui->line_trade_for->setEnabled(false);

    connect(ui->radio_standard, &QRadioButton::toggled, this, &TrocadorAppWidget::onRadioButtonToggled);
    connect(ui->radio_payment, &QRadioButton::toggled, this, &TrocadorAppWidget::onRadioButtonToggled);

    ui->combo_trade_for->addItems(config()->get(Config::trocadorAppCryptoSymbols).toStringList());
    ui->combo_trade_for->setCurrentText(config()->get(Config::trocadorAppCryptoSymbols).toStringList().filter("BTC").first());

    m_network = new Networking(this);
    m_api = new TrocadorAppApi(this, m_network);

    m_model = new TrocadorAppModel(this);
    ui->treeView->setModel(m_model);

    ui->treeView->header()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &TrocadorAppWidget::showContextMenu);

    connect(ui->btn_search, &QPushButton::clicked, this, &TrocadorAppWidget::onSearchClicked);
    connect(m_api, &TrocadorAppApi::ApiResponse, this, &TrocadorAppWidget::onApiResponse);
}

void TrocadorAppWidget::onRadioButtonToggled(){
    if (ui->radio_standard->isChecked()){
        ui->line_amount->setEnabled(true);
        ui->line_amount->setPlaceholderText("Amount you send");
        ui->line_trade_for->clear();
        ui->line_trade_for->setEnabled(false);
        ui->line_trade_for->setPlaceholderText("To trade for");
    }else if (ui->radio_payment->isChecked()){
        ui->line_amount->clear();
        ui->line_amount->setEnabled(false);
        ui->line_amount->setPlaceholderText("To be paid in");
        ui->line_trade_for->setEnabled(true);
        ui->line_trade_for->setPlaceholderText("Amount you receive");
    }
}

void TrocadorAppWidget::onSearchClicked() {
    m_model->clearData();
    ui->btn_search->setEnabled(false);

    this->searchOffers();
}

void TrocadorAppWidget::searchOffers() {
    QString currencyCode = ui->combo_currency->currentText();
    QString tradeForCode = ui->combo_trade_for->currentText();
    QString networkFrom = "Mainnet";
    QString networkTo = "Mainnet";

    if(currencyCode == "USDC" || currencyCode == "USDT")
        networkFrom = "ERC20";
    else if (tradeForCode == "USDC" || tradeForCode == "USDT")
        networkTo = "ERC20";

    if (ui->radio_standard->isChecked()){
        QString amountFrom = ui->line_amount->text();
        QString paymentMethod = "False";      
        m_api->requestStandard(currencyCode, networkFrom, tradeForCode, networkTo, amountFrom, paymentMethod);
    }
    else if (ui->radio_payment->isChecked()){
        QString amountTo = ui->line_trade_for->text();
        QString paymentMethod = "True";
        m_api->requestPayment(currencyCode, networkFrom, tradeForCode, networkTo, amountTo, paymentMethod);
    }
}

void TrocadorAppWidget::onApiResponse(const TrocadorAppApi::TrocadorAppResponse &resp) {
    ui->btn_search->setEnabled(true);

    if (!resp.ok) {
        QMessageBox::warning(this, "TrocadorApp error", QString("Request failed:\n\n%1").arg(resp.message));
        return;
    }

    if (resp.endpoint == TrocadorAppApi::REQUEST_STANDARD
        || resp.endpoint == TrocadorAppApi::REQUEST_PAYMENT)
    {
        m_model->addData(resp.obj["quotes"].toObject()["quotes"].toArray());
        m_model->addTradeId(resp.obj["trade_id"].toString());
    }
}

void TrocadorAppWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction("Go to offer", this, &TrocadorAppWidget::openOfferUrl);
    menu.exec(ui->treeView->viewport()->mapToGlobal(point));
}

void TrocadorAppWidget::openOfferUrl() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QString tradeId = m_model->getTradeId();
    QString frontend = config()->get(Config::trocadorAppFrontend).toString();
    QString offerUrl = QString("%1/exchange/%2").arg(frontend, tradeId);
    Utils::externalLinkWarning(this, offerUrl);
}

TrocadorAppWidget::~TrocadorAppWidget() = default;