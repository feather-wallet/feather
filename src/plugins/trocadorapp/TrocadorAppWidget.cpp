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
        ui->line_trade_for->clear();
        ui->line_trade_for->setEnabled(false);
        ui->line_trade_for->setPlaceholderText("To trade for");
    }else if (ui->radio_payment->isChecked()){
        ui->combo_currency->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("BTC").first());
        ui->combo_trade_for->setCurrentText(config()->get(Config::cryptoSymbols).toStringList().filter("XMR").first());
        ui->line_amount->clear();
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

void TrocadorAppWidget::searchOffers() {
    QString currencyCode = ui->combo_currency->currentText();
    QString tradeForCode = ui->combo_trade_for->currentText();

    if (ui->radio_standard->isChecked()){
        QString amountFrom = ui->line_amount->text();
        QString paymentMethod = "False";
        m_api->requestStandard(currencyCode, "Mainnet", tradeForCode, "Mainnet", amountFrom, paymentMethod);
    }
    else if (ui->radio_payment->isChecked()){
        QString amountTo = ui->line_trade_for->text();
        QString paymentMethod = "True";
        m_api->requestPayment(currencyCode, "Mainnet", tradeForCode, "Mainnet", amountTo, paymentMethod);
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
        bool hasNextPage = resp.obj["pagination"].toObject().contains("next");
        ui->frame_loadMore->setVisible(hasNextPage);

        m_model->addData(resp.obj["data"].toObject()["ad_list"].toArray());
    }
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