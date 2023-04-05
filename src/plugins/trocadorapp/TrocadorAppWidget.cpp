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
    
    ui->combo_currency->addItems(config()->get(Config::cryptoSymbols).toStringList());
    ui->combo_currency->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("XMR").first());
    ui->line_trade_for->setEnabled(false);

    connect(ui->radio_standard, &QRadioButton::toggled, this, &TrocadorAppWidget::onRadioButtonToggled);
    connect(ui->radio_payment, &QRadioButton::toggled, this, &TrocadorAppWidget::onRadioButtonToggled);

    ui->combo_trade_for->addItems(config()->get(Config::cryptoSymbols).toStringList());
    ui->combo_trade_for->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("BTC").first());


    m_network = new Networking(this);
    m_api = new TrocadorAppApi(this, m_network);

    m_model = new TrocadorAppModel(this);
    ui->treeView->setModel(m_model);

    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(TrocadorAppModel::Spread, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);

    connect(ui->treeView, &QTreeView::doubleClicked, this, &TrocadorAppWidget::viewOfferDetails);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &TrocadorAppWidget::showContextMenu);

    connect(ui->btn_search, &QPushButton::clicked, this, &TrocadorAppWidget::onSearchClicked);
    connect(m_api, &TrocadorAppApi::ApiResponse, this, &TrocadorAppWidget::onApiResponse);
    connect(ui->btn_loadMore, &QPushButton::clicked, this, &TrocadorAppWidget::onLoadMore);

    connect(websocketNotifier(), &WebsocketNotifier::LocalMoneroCurrenciesReceived, this, &TrocadorAppWidget::onWsCurrenciesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::LocalMoneroPaymentMethodsReceived, this, &TrocadorAppWidget::onWsPaymentMethodsReceived);

    ui->frame_loadMore->hide();

    QTimer::singleShot(1, [this]{
        this->skinChanged();
    });
}

void TrocadorAppWidget::onRadioButtonToggled(){
    if (ui->radio_standard->isChecked()){
        ui->combo_currency->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("XMR").first());
        ui->combo_trade_for->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("BTC").first());
        ui->line_amount->setEnabled(true);
        ui->line_amount->setPlaceholderText("Amount you send");
        ui->line_trade_for->setEnabled(false);
        ui->line_trade_for->setPlaceholderText("To trade for");
    }else if (ui->radio_payment->isChecked()){
        ui->combo_currency->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("BTC").first());
        ui->combo_trade_for->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("XMR").first());
        ui->line_amount->setEnabled(false);
        ui->line_amount->setPlaceholderText("To be paid in");
        ui->line_trade_for->setEnabled(true);
        ui->line_trade_for->setPlaceholderText("Amount you receive");
    }
}

void TrocadorAppWidget::skinChanged() {
    
}


void TrocadorAppWidget::onSearchClicked() {
    m_model->clearData();
    m_currentPage = 0;
    ui->btn_search->setEnabled(false);

    this->searchOffers();
}

void TrocadorAppWidget::onApiResponse(const TrocadorAppApi::TrocadorAppResponse &resp) {
    ui->btn_search->setEnabled(true);

    if (!resp.ok) {
        QMessageBox::warning(this, "TrocadorApp error", QString("Request failed:\n\n%1").arg(resp.message));
        return;
    }

    if (resp.endpoint == TrocadorAppApi::BUY_MONERO_ONLINE
        || resp.endpoint == TrocadorAppApi::SELL_MONERO_ONLINE)
    {
        bool hasNextPage = resp.obj["pagination"].toObject().contains("next");
        ui->frame_loadMore->setVisible(hasNextPage);

        m_model->addData(resp.obj["data"].toObject()["ad_list"].toArray());
    }
    else if (resp.endpoint == TrocadorAppApi::SPREAD) {
        m_model->setPaymentMethods(resp.obj["data"].toObject()["methods"].toObject());
    }
}

void TrocadorAppWidget::onLoadMore() {
    m_currentPage += 1;
    this->searchOffers(m_currentPage);
}

void TrocadorAppWidget::onWsCurrenciesReceived(const QJsonArray &currencies) {
    QString currentText = ui->combo_currency->currentText();

    ui->combo_currency->clear();
    for (const auto currency : currencies) {
        ui->combo_currency->addItem(currency.toString());
    }

    // restore previous selection
    int index = ui->combo_currency->findText(currentText);
    ui->combo_currency->setCurrentIndex(index);
}

void TrocadorAppWidget::onWsPaymentMethodsReceived(const QJsonObject &payment_methods) {
    m_paymentMethods = payment_methods;
    m_model->setPaymentMethods(payment_methods);
}

void TrocadorAppWidget::searchOffers(int page) {
    QString amount = ui->line_amount->text();
    QString currencyCode = ui->combo_currency->currentText();
    QString paymentMethod = ui->combo_trade_for->currentData().toString();

    if (ui->radio_standard->isChecked())
        m_api->buyMoneroOnline(currencyCode, paymentMethod, amount, page);
    else if (ui->radio_payment->isChecked())
        m_api->sellMoneroOnline(currencyCode, paymentMethod, amount, page);
}

void TrocadorAppWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction("Go to offer", this, &TrocadorAppWidget::openOfferUrl);
    menu.addAction("View offer details", this, &TrocadorAppWidget::viewOfferDetails);
    menu.exec(ui->treeView->viewport()->mapToGlobal(point));
}

void TrocadorAppWidget::openOfferUrl() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QJsonObject offerData = m_model->getOffer(index.row());
    QString frontend = config()->get(Config::localMoneroFrontend).toString();

    QString offerUrl = QString("%1/ad/%2").arg(frontend, offerData["data"].toObject()["ad_id"].toString());

    Utils::externalLinkWarning(this, offerUrl);
}

void TrocadorAppWidget::viewOfferDetails() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QJsonObject offerData = m_model->getOffer(index.row());
    QString details = offerData["data"].toObject()["msg"].toString();
    details.remove("*");

    if (details.isEmpty()) {
        details = "No details.";
    }

    TrocadorAppInfoDialog dialog(this, m_model, index.row());
    dialog.exec();
}

TrocadorAppWidget::~TrocadorAppWidget() = default;