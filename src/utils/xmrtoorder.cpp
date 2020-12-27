// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "xmrtoorder.h"

#include <utility>

#include "libwalletqt/Wallet.h"
#include "appcontext.h"
#include "globals.h"
#include "utils/xmrto.h"

XmrToOrder::XmrToOrder(AppContext *ctx, UtilsNetworking *network, QString baseUrl, bool clearnet, XmrToRates *rates, QObject *parent) :
        QObject(parent),
        m_ctx(ctx),
        m_network(network),
        m_baseUrl(std::move(baseUrl)),
        m_rates(rates),
        m_clearnet(clearnet) {
    this->state = OrderState::Status_Idle;

    m_baseUrl = m_ctx->networkType == NetworkType::Type::MAINNET ? "https://xmr.to" : "https://test.xmr.to";
    m_api = new XmrToApi(this, network, m_baseUrl);

    connect(m_api, &XmrToApi::ApiResponse, this, &XmrToOrder::onApiResponse);
    connect(m_ctx, &AppContext::transactionCommitted, this, &XmrToOrder::onTransactionCommitted);
    connect(m_ctx, &AppContext::createTransactionCancelled, this, &XmrToOrder::onTransactionCancelled);
}

void XmrToOrder::onTransactionCancelled(const QString &address, double amount) {
    // listener for all cancelled transactions - will try to match the exact amount to this order.
    if(this->incoming_amount_total != amount || this->receiving_subaddress != address) return;

    this->errorMsg = "TX cancelled by user";
    this->changeState(OrderState::Status_OrderFailed);
    this->stop();
}

void XmrToOrder::onTransactionCommitted(bool status, PendingTransaction *tx, const QStringList& txid) {
    // listener for all outgoing transactions - will try to match the exact amount to this order.
    if(this->state == OrderState::Status_OrderUnpaid){
        if(tx->amount() / globals::cdiv == this->incoming_amount_total) {
            if(!status) {
                this->errorMsg = "TX failed to commit";
                this->changeState(OrderState::Status_OrderFailed);
                this->stop();
                return;
            }

            this->xmr_txid = txid.at(0);
            this->m_paymentSent = true;
            this->changeState(OrderState::Status_OrderXMRSent);
        }
    }
}

void XmrToOrder::onApiFailure(const XmrToResponse &resp) {
    this->errorCode = resp.error.code;
    this->errorMsg = resp.message;

    switch (resp.endpoint) {
        case ORDER_CREATE:
            this->onCreatedError();
            break;
        case ORDER_STATUS:
            this->onCheckedError(resp.error);
            break;
        default:
            return;
    }
}

void XmrToOrder::onApiResponse(const XmrToResponse& resp) {
    if (!resp.ok) {
        this->onApiFailure(resp);
        return;
    }

    switch (resp.endpoint) {
        case ORDER_CREATE:
            this->onCreated(resp.obj);
            break;
        case ORDER_STATUS:
            this->onChecked(resp.obj);
            break;
        default:
            return;
    }
}

void XmrToOrder::create(double amount, const QString &currency, const QString &btcAddress) {
    if(this->m_ctx->currentWallet == nullptr) {
        this->errorMsg = "No wallet opened";
        this->changeState(OrderState::Status_OrderFailed);
        return;
    }

    m_api->createOrder(amount, currency, btcAddress);
    this->changeState(OrderState::Status_OrderCreating);
}

void XmrToOrder::onCreatedError() {
    this->changeState(OrderState::Status_OrderFailed);
}

void XmrToOrder::onCreated(const QJsonObject &object) {
    if(!object.contains("state"))
        this->errorMsg = "Could not parse 'state' from JSON response";
    if(object.value("state").toString() != "TO_BE_CREATED")
        this->errorMsg = "unknown state from response, should be \"TO_BE_CREATED\"";

    if(!this->errorMsg.isEmpty()) {
        this->changeState(OrderState::Status_OrderFailed);
        return;
    }

    if(m_created) return;
    m_created = true;
    this->btc_amount = object.value("btc_amount").toDouble();
    this->btc_dest_address = object.value("btc_dest_address").toString();
    this->uses_lightning = object.value("uses_lightning").toBool();
    this->uuid = object.value("uuid").toString();
    m_checkTimer.start(1000*5);
    m_countdownTimer.start(1000);
    connect(&m_checkTimer, &QTimer::timeout, this, &XmrToOrder::check);
    connect(&m_countdownTimer, &QTimer::timeout, this, &XmrToOrder::onCountdown);

    this->changeState(OrderState::Status_OrderToBeCreated);
    this->check();
}

void XmrToOrder::check() {
    if(this->m_ctx->currentWallet == nullptr)
        return;

    m_api->getOrderStatus(this->uuid);
}

void XmrToOrder::onCheckedError(const XmrToError& err) {
    if (!err.code.isEmpty())
        this->changeState(OrderState::Status_OrderFailed);

    m_checkFailures += 1;
    if(m_checkFailures > 15){
        this->errorMsg = "Too many failed attempts";
        this->changeState(OrderState::Status_OrderFailed);
    }
}

void XmrToOrder::onChecked(const QJsonObject &object) {
    if(object.contains("btc_amount"))
        this->btc_amount = object.value("btc_amount").toString().toDouble();
    if(object.contains("btc_dest_address"))
        this->btc_dest_address = object.value("btc_dest_address").toString();
    if(object.contains("seconds_till_timeout")) {
        this->seconds_till_timeout = object.value("seconds_till_timeout").toInt();
        this->countdown = this->seconds_till_timeout;
    }
    if(object.contains("created_at"))
        this->created_at = object.value("created_at").toString();
    if(object.contains("expires_at"))
        this->expires_at = object.value("expires_at").toString();
    if(object.contains("incoming_amount_total"))
        this->incoming_amount_total = object.value("incoming_amount_total").toString().toDouble();
    if(object.contains("remaining_amount_incoming"))
        this->remaining_amount_incoming = object.value("remaining_amount_incoming").toString().toDouble();
    if(object.contains("incoming_price_btc"))
    {
        qDebug() << object.value("incoming_price_btc").toString();
        this->incoming_price_btc = object.value("incoming_price_btc").toString().toDouble();
    }
    if(object.contains("receiving_subaddress"))
        this->receiving_subaddress = object.value("receiving_subaddress").toString();

    if(object.contains("payments")) {
        // detect btc txid, xmr.to api can output several - we'll just grab the first #yolo
        auto payments = object.value("payments").toArray();
        for(const auto &payment: payments){
            auto obj = payment.toObject();
            if(obj.contains("tx_id")) {
                this->btc_txid = obj.value("tx_id").toString();
                break;
            }
        }
    }

    this->changeState(object.value("state").toString());
}

void XmrToOrder::changeState(const QString &_state) {
    for(const auto &key: XmrTo::stateMap.keys()) {
        const auto &val = XmrTo::stateMap[key];
        if(_state == val){
            this->changeState(key);
            return;
        }
    }
}

void XmrToOrder::changeState(OrderState _state) {
    if(this->m_ctx->currentWallet == nullptr)
        return;

    if(_state == OrderState::Status_OrderUnderPaid && m_paymentSent) {
        this->state = OrderState::Status_OrderXMRSent;
        emit orderChanged();
        return;
    }

    if(_state == this->state) return;
    switch(_state){
        case OrderState::Status_Idle:
            break;
        case OrderState::Status_OrderCreating:
            break;
        case OrderState::Status_OrderToBeCreated:
            break;
        case OrderState::Status_OrderUnderPaid:
            emit orderFailed(this);
            this->stop();
            break;
        case OrderState::Status_OrderUnpaid:
            // need to send Monero
            if(!m_paymentRequested) {
                auto unlocked_balance = m_ctx->currentWallet->unlockedBalance() / globals::cdiv;
                if (this->incoming_amount_total >= unlocked_balance) {
                    this->state = OrderState::Status_OrderFailed;
                    emit orderFailed(this);
                    this->stop();
                    break;
                }
                m_paymentRequested = true;
                emit orderPaymentRequired(this);
            }
            break;
        case OrderState::Status_OrderFailed:
            emit orderFailed(this);
            this->stop();
            break;
        case OrderState::Status_OrderPaidUnconfirmed:
            emit orderPaidUnconfirmed(this);
            break;
        case OrderState::Status_OrderPaid:
            emit orderPaid(this);
            break;
        case OrderState::Status_OrderTimedOut:
            emit orderFailed(this);
            this->stop();
            break;
        case OrderState::Status_OrderBTCSent:
            emit orderPaid(this);
            this->stop();
            break;
        default:
            break;
    }

    this->state = _state;
    emit orderChanged();
}

void XmrToOrder::onCountdown() {
    if(this->countdown <= 0) return;
    this->countdown -= 1;
    emit orderChanged();
}

void XmrToOrder::stop(){
    this->m_checkTimer.stop();
    this->m_countdownTimer.stop();
}

XmrToOrder::~XmrToOrder(){
    this->stop();
    this->disconnect();
    this->m_network->deleteLater();
}