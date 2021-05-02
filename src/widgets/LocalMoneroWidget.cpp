// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "LocalMoneroWidget.h"
#include "ui_LocalMoneroWidget.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/NetworkManager.h"
#include "utils/WebsocketNotifier.h"
#include "dialog/LocalMoneroInfoDialog.h"

#include <QMessageBox>
#include <QMenu>

LocalMoneroWidget::LocalMoneroWidget(QWidget *parent, AppContext *ctx)
    : QWidget(parent)
    , ui(new Ui::LocalMoneroWidget)
    , m_ctx(ctx)
{
    ui->setupUi(this);

//    this->adjustSize();

    QPixmap logo(":/assets/images/localMonero_logo.png");
    ui->logo->setPixmap(logo.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->combo_currency->addItem(config()->get(Config::preferredFiatCurrency).toString());

    m_network = new UtilsNetworking(getNetworkTor(), this);
    m_api = new LocalMoneroApi(this, m_network);

    m_model = new LocalMoneroModel(this);
    ui->treeView->setModel(m_model);

    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(LocalMoneroModel::PaymentMethodDetail, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);

    connect(ui->treeView, &QTreeView::doubleClicked, this, &LocalMoneroWidget::viewOfferDetails);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &LocalMoneroWidget::showContextMenu);

    connect(ui->btn_search, &QPushButton::clicked, this, &LocalMoneroWidget::onSearchClicked);
    connect(ui->btn_signUp, &QPushButton::clicked, this, &LocalMoneroWidget::onSignUpClicked);
    connect(m_api, &LocalMoneroApi::ApiResponse, this, &LocalMoneroWidget::onApiResponse);
    connect(ui->btn_loadMore, &QPushButton::clicked, this, &LocalMoneroWidget::onLoadMore);

    connect(websocketNotifier(), &WebsocketNotifier::LocalMoneroCountriesReceived, this, &LocalMoneroWidget::onWsCountriesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::LocalMoneroCurrenciesReceived, this, &LocalMoneroWidget::onWsCurrenciesReceived);
    connect(websocketNotifier(), &WebsocketNotifier::LocalMoneroPaymentMethodsReceived, this, &LocalMoneroWidget::onWsPaymentMethodsReceived);

    connect(ui->combo_currency, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LocalMoneroWidget::updatePaymentMethods);

    ui->frame_loadMore->hide();

    this->skinChanged();
}

void LocalMoneroWidget::skinChanged() {
    if (ColorScheme::hasDarkBackground(this)) {
        ui->radio_buy->setIcon(icons()->icon("localMonero_buy_white.svg"));
        ui->radio_sell->setIcon(icons()->icon("localMonero_sell_white.svg"));
    } else {
        ui->radio_buy->setIcon(icons()->icon("localMonero_buy.svg"));
        ui->radio_sell->setIcon(icons()->icon("localMonero_sell.svg"));
    }
}

void LocalMoneroWidget::onSearchClicked() {
    m_model->clearData();
    m_currentPage = 0;

    this->searchOffers();
}

void LocalMoneroWidget::onSignUpClicked() {
    QString signupUrl = QString("%1/signup").arg(config()->get(Config::localMoneroFrontend).toString());
    Utils::externalLinkWarning(this, signupUrl);
}

void LocalMoneroWidget::onApiResponse(const LocalMoneroApi::LocalMoneroResponse &resp) {
    qDebug() << "We got a response";

    if (!resp.ok) {
        QMessageBox::warning(this, "LocalMonero error", QString("Request failed:\n\n%1").arg(resp.message));
        return;
    }

    if (resp.endpoint == LocalMoneroApi::BUY_MONERO_ONLINE
        || resp.endpoint == LocalMoneroApi::SELL_MONERO_ONLINE)
    {
        bool hasNextPage = resp.obj["pagination"].toObject().contains("next");
        ui->frame_loadMore->setVisible(hasNextPage);

        m_model->addData(resp.obj["data"].toObject()["ad_list"].toArray());
    }
    else if (resp.endpoint == LocalMoneroApi::PAYMENT_METHODS) {
        m_model->setPaymentMethods(resp.obj["data"].toObject()["methods"].toObject());
    }
}

void LocalMoneroWidget::onLoadMore() {
    m_currentPage += 1;
    this->searchOffers(m_currentPage);
}

void LocalMoneroWidget::onWsCountriesReceived(const QJsonArray &countries) {
    ui->combo_country->clear();
    ui->combo_country->addItem("Any country");
    for (const auto country : countries) {
        ui->combo_country->addItem(country[0].toString(), country[1].toString());
    }
}

void LocalMoneroWidget::onWsCurrenciesReceived(const QJsonArray &currencies) {
    QString currentText = ui->combo_currency->currentText();

    ui->combo_currency->clear();
    for (const auto currency : currencies) {
        ui->combo_currency->addItem(currency.toString());
    }

    // restore previous selection
    int index = ui->combo_currency->findText(currentText);
    ui->combo_currency->setCurrentIndex(index);
}

void LocalMoneroWidget::onWsPaymentMethodsReceived(const QJsonObject &payment_methods) {
    m_paymentMethods = payment_methods;
    m_model->setPaymentMethods(payment_methods);
    this->updatePaymentMethods();
}

void LocalMoneroWidget::searchOffers(int page) {
    QString amount = ui->line_amount->text();
    QString currencyCode = ui->combo_currency->currentText();
    QString countryCode = ui->combo_country->currentData().toString();
    QString paymentMethod = ui->combo_paymentMethod->currentData().toString();

    if (ui->radio_buy->isChecked())
        m_api->buyMoneroOnline(currencyCode, countryCode, paymentMethod, amount, page);
    else if (ui->radio_sell->isChecked())
        m_api->sellMoneroOnline(currencyCode, countryCode, paymentMethod, amount, page);
}

LocalMoneroWidget::~LocalMoneroWidget() {
    delete ui;
}

void LocalMoneroWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    menu.addAction("Go to offer", this, &LocalMoneroWidget::openOfferUrl);
    menu.addAction("View offer details", this, &LocalMoneroWidget::viewOfferDetails);
    menu.exec(ui->treeView->viewport()->mapToGlobal(point));
}

void LocalMoneroWidget::openOfferUrl() {
    QModelIndex index = ui->treeView->currentIndex();
    if (!index.isValid()) {
        return;
    }

    QJsonObject offerData = m_model->getOffer(index.row());
    QString frontend = config()->get(Config::localMoneroFrontend).toString();

    QString offerUrl = QString("%1/ad/%2").arg(frontend, offerData["data"].toObject()["ad_id"].toString());

    Utils::externalLinkWarning(this, offerUrl);
}

void LocalMoneroWidget::viewOfferDetails() {
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

    LocalMoneroInfoDialog dialog(this, m_model, index.row());
    dialog.exec();
}

void LocalMoneroWidget::updatePaymentMethods() {
    QString currency = ui->combo_currency->currentText().toUpper();

    ui->combo_paymentMethod->clear();
    ui->combo_paymentMethod->addItem("All online offers");

    for (const auto &payment_method : m_paymentMethods.keys()) {
        auto pm = m_paymentMethods[payment_method].toObject();

        if (pm["currencies"].toArray().contains(currency)) {
            QString name = pm["name"].toString();
            if (name.isEmpty())
                name = payment_method;
            ui->combo_paymentMethod->addItem(name, payment_method);
        }
    }
}