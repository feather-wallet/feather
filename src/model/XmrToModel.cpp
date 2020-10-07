// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "XmrToModel.h"
#include "model/ModelUtils.h"
#include "utils/xmrto.h"

XmrToModel::XmrToModel(QList<XmrToOrder*> *orders, QObject *parent)
        : QAbstractTableModel(parent),
          orders(orders)
{
}

void XmrToModel::update() {
    beginResetModel();
    endResetModel();
}

int XmrToModel::rowCount(const QModelIndex &) const {
    return this->orders->count();
}

int XmrToModel::columnCount(const QModelIndex &) const {
    return COUNT;
}

QVariant XmrToModel::data(const QModelIndex &index, int role) const {
    const int _row = index.row();
    const int _col = index.column();
    const auto order = this->orders->at(_row);

    if (role == Qt::DisplayRole){
        switch(index.column()){
            case Status:
            {
                QString status = XmrTo::stateMap[(OrderState) order->state];

                if (order->state == OrderState::Status_OrderUnpaid)
                    return QString("%1 (%2)").arg(status, QString::number(order->countdown));

                return status;
            }
            case ID:
                return !order->uuid.isEmpty() ? order->uuid.split("-")[1] : "-";
            case Destination:
                return ModelUtils::displayAddress(order->btc_dest_address, 1);
            case Conversion:
                if(order->state <= OrderState::Status_OrderToBeCreated)
                    return "";

                return QString("%1 XMR ⟶ %2 BTC").arg(QString::number(order->incoming_amount_total), QString::number(order->btc_amount));
            case Rate:
                return order->incoming_price_btc ? QString::number(order->incoming_price_btc, 'f', 6) : "";
            case ErrorMsg:
                if(order->errorMsg.isEmpty()) return "";
                return order->errorMsg;
            default: return {};
        }
    }

    else if(role == Qt::BackgroundRole) {
        if (_col == 0) {
            if (order->state == OrderState::Status_OrderPaid || order->state == OrderState::Status_OrderPaidUnconfirmed)
                return QBrush(Qt::darkGreen);
            else if (order->state == OrderState::Status_OrderCreating || order->state == OrderState::Status_OrderToBeCreated)
                return QBrush(Qt::yellow);
            else if (order->state == OrderState::Status_OrderUnpaid)
                return QBrush(Qt::cyan);
            else if (order->state == OrderState::Status_OrderBTCSent)
                return QBrush(Qt::green);
            else if (order->state == OrderState::Status_OrderFailed || order->state == OrderState::Status_OrderTimedOut)
                return QBrush(QColor(191, 255, 0)); // lime
        }
    }
    else if (role == Qt::FontRole) {
        switch(index.column()) {
            case ID:
            case Destination:
                return ModelUtils::getMonospaceFont();
        }
    }

    return QVariant();
}

QVariant XmrToModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Status:
                return QString("Status");
            case ID:
                return QString("ID");
            case Destination:
                return QString("Address");
            case Conversion:
                return QString("Conversion");
            case Rate:
                return QString("Rate");
            case ErrorMsg:
                return QString("Message");
            default:
                return QVariant();
        }
    }
    return QVariant();
}