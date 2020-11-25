// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "MorphTokenWidget.h"
#include "ui_MorphTokenWidget.h"
#include "mainwindow.h"

#include <QMessageBox>

MorphTokenWidget::MorphTokenWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MorphTokenWidget)
{
    ui->setupUi(this);
    m_ctx = MainWindow::getContext();

    m_network = new UtilsNetworking(this->m_ctx->network);
    m_api = new MorphTokenApi(this, m_network);

    connect(ui->btnCreateTrade, &QPushButton::clicked, this, &MorphTokenWidget::createTrade);
    connect(ui->btn_lookupTrade, &QPushButton::clicked, this, &MorphTokenWidget::lookupTrade);
    connect(ui->btn_getRates, &QPushButton::clicked, this, &MorphTokenWidget::getRates);

    connect(m_api, &MorphTokenApi::ApiResponse, this, &MorphTokenWidget::onApiResponse);

    connect(ui->combo_From, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){
        this->displayRate();
        ui->label_refundAddress->setText(QString("Refund address (%1):").arg(ui->combo_From->currentText()));
    });
    connect(ui->combo_To, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){
        this->displayRate();
        ui->label_destinationAddress->setText(QString("Destination address (%1):").arg(ui->combo_To->currentText()));
    });

    connect(ui->check_autorefresh, &QCheckBox::toggled, [this](bool toggled){
        m_countdown = 30;
        toggled ? m_countdownTimer.start(1000) : m_countdownTimer.stop();
        ui->check_autorefresh->setText("Autorefresh");
    });
    connect(&m_countdownTimer, &QTimer::timeout, this, &MorphTokenWidget::onCountdown);

    connect(ui->line_Id, &QLineEdit::textChanged, [this](const QString &text){
        ui->btn_lookupTrade->setEnabled(!text.isEmpty());
        ui->check_autorefresh->setEnabled(!text.isEmpty());
    });

    // Default to BTC -> XMR
    ui->combo_From->setCurrentIndex(1);
    ui->combo_To->setCurrentIndex(0);

    ui->label_rate->setVisible(false);
    m_ratesTimer.setSingleShot(true);
    connect(&m_ratesTimer, &QTimer::timeout, [this]{
        ui->label_rate->setVisible(false);
    });

    ui->tabWidget->setTabVisible(2, false);
}

void MorphTokenWidget::createTrade() {
    QString inputAsset = ui->combo_From->currentText();
    QString outputAsset = ui->combo_To->currentText();
    QString refundAddress = ui->line_refundAddress->text();
    QString destinationAddress = ui->line_destinationAddress->text();

    m_api->createTrade(inputAsset, outputAsset, refundAddress, destinationAddress);
}

void MorphTokenWidget::lookupTrade() {
    QString morphId = ui->line_Id->text();

    if (!morphId.isEmpty())
        m_api->getTrade(morphId);
}

void MorphTokenWidget::getRates() {
    m_api->getRates();
}

void MorphTokenWidget::onApiResponse(const MorphTokenApi::MorphTokenResponse &resp) {
    if (!resp.ok) {
        ui->check_autorefresh->setChecked(false);
        QMessageBox::warning(this, "MorphToken error", QString("Request failed:\n\n%1").arg(resp.message));
        return;
    }

    ui->debugInfo->setPlainText(QJsonDocument(resp.obj).toJson(QJsonDocument::Indented));

    if (resp.endpoint == MorphTokenApi::Endpoint::CREATE_TRADE || resp.endpoint == MorphTokenApi::Endpoint::GET_TRADE) {
        ui->tabWidget->setCurrentIndex(1);
        ui->line_Id->setText(resp.obj.value("id").toString());

        auto obj = resp.obj;
        auto input = obj["input"].toObject();
        auto output = obj["output"].toArray()[0].toObject();
        QString state = obj.value("state").toString();
        QString statusText;

        ui->trade->setTitle(QString("Trade (%1)").arg(state));

        statusText += QString("Morph ID: %1\n\n").arg(obj["id"].toString());

        if (state == "PENDING") {
            statusText += QString("Waiting for a deposit, send %1 to %2\n").arg(input["asset"].toString(),
                                                                                input["deposit_address"].toString());
            statusText += QString("Rate: 1 %1 -> %2 %3\n\n").arg(input["asset"].toString(),
                                                                 output["seen_rate"].toString(),
                                                                 output["asset"].toString());
            statusText += "Limits:\n";
            statusText += QString("  Minimum amount accepted: %1 %2\n").arg(formatAmount(input["asset"].toString(), input["limits"].toObject()["min"].toDouble()),
                                                                            input["asset"].toString());
            statusText += QString("  Maximum amount accepted: %1 %2\n").arg(formatAmount(input["asset"].toString(), input["limits"].toObject()["max"].toDouble()),
                                                                            input["asset"].toString());
            statusText += QString("\nSend a single deposit. If the amount is outside the limits, a refund will happen.");
        } else if (state == "PROCESSING" || state == "TRADING" || state == "CONFIRMING") {
            if (state == "CONFIRMING") {
                statusText += QString("Waiting for confirmations\n");
            } else if (state == "TRADING") {
                statusText += QString("Your transaction has been received and is confirmed. MorphToken is now executing your trade.\n"
                                      "Usually this step takes no longer than a minute, "
                                      "but in rare cases it can take a couple hours.\n"
                                      "Wait a bit before contacting support.\n");
            }
            statusText += QString("Converting %1 to %2\n").arg(input["asset"].toString(), output["asset"].toString());
            statusText += QString("Sending to %1\n").arg(output["address"].toString());
            statusText += QString("Stuck? Contact support at contact@morphtoken.com");
        } else if (state == "COMPLETE") {
            if (output["txid"].toString().isEmpty()) {
                statusText += QString("MorphToken is sending your transaction.\n");
                statusText += QString("MorphToken will send %1 %2 to %2").arg(this->formatAmount(output["asset"].toString(), output["converted_amount"].toDouble() - output["network_fee"].toObject()["fee"].toDouble()),
                                                                              output["asset"].toString(),
                                                                              output["address"].toString());
            } else {
                statusText += QString("Sent %1 %2 to %3\ntxid: {}").arg(this->formatAmount(output["asset"].toString(), output["converted_amount"].toDouble() - output["network_fee"].toObject()["fee"].toDouble()),
                                                                        output["asset"].toString(),
                                                                        output["address"].toString(),
                                                                        output["txid"].toString());
            }
        } else if (state == "PROCESSING_REFUND" || state == "COMPLETE_WITH_REFUND") {
            statusText += QString("MorphToken will refund %1 %2\nReason: %3\n").arg(obj["final_amount"].toString(),
                                                                                  obj["asset"].toString(),
                                                                                  obj["reason"].toString());

            if (obj.contains("txid")) {
                statusText += QString("txid: %1").arg(obj["txid"].toString());
            }
        } else if (state == "COMPLETE_WITHOUT_REFUND") {
            statusText += "Deposit amount below network fee, too small to refund.";
        }

        ui->label_status->setText(statusText);
    } else if (resp.endpoint == MorphTokenApi::Endpoint::GET_RATES) {
        m_rates = resp.obj.value("data").toObject();
        this->displayRate();
        ui->label_rate->setVisible(true);
        m_ratesTimer.start(120 * 1000);
    }

    if (resp.endpoint == MorphTokenApi::Endpoint::CREATE_TRADE) {
        QMessageBox::information(this, "MorphToken", "Trade created!\n\nMake sure to save your Morph ID. You may need it in case something goes wrong.");
    }
}

void MorphTokenWidget::onCountdown() {
    if (m_countdown > 0) {
        m_countdown -= 1;
    } else {
        this->lookupTrade();
        m_countdown = 30;
    }
    ui->check_autorefresh->setText(QString("Autorefresh (%1)").arg(m_countdown));
}

void MorphTokenWidget::displayRate() {
    QString inputAsset = ui->combo_From->currentText();
    QString outputAsset = ui->combo_To->currentText();
    QString outputRate = m_rates.value(inputAsset).toObject().value(outputAsset).toString("1");

    QString rateStr = QString("1 %1 -> %2 %3").arg(inputAsset, outputRate, outputAsset);
    ui->label_rate->setText(rateStr);
}

QString MorphTokenWidget::formatAmount(const QString &asset, double amount) {
    double displayAmount;
    double div;

    if (asset == "ETH")
        div = 1e18;
    else if (asset == "XMR")
        div = 1e12;
    else
        div = 1e8;

    displayAmount = amount / div;

    return QString::number(displayAmount, 'f', 8);
}

MorphTokenWidget::~MorphTokenWidget() {
    delete ui;
}
