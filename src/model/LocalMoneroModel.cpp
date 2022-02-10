// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "LocalMoneroModel.h"
#include <QColor>
#include "utils/Utils.h"

LocalMoneroModel::LocalMoneroModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int LocalMoneroModel::rowCount(const QModelIndex &parent) const {
    return m_data.count();
}

int LocalMoneroModel::columnCount(const QModelIndex &parent) const {
    return Column::COUNT;
}

QVariant LocalMoneroModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Seller:
                return QString("Seller");
            case Country:
                return QString("Country");
            case PaymentMethod:
                return QString("Payment Method");
            case PaymentMethodDetail:
                return QString("Detail");
            case PriceXMR:
                return QString("Price/XMR");
            case Limits:
                return QString("Limits");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant LocalMoneroModel::data(const QModelIndex &index, int role) const {
    const int col = index.column();
    const auto row = m_data.at(index.row()).toObject()["data"].toObject();

    if (row.isEmpty()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (col) {
            case Column::Seller: {
                auto seller = row["profile"].toObject();
                return seller["name"].toString();
                // TODO: online indicator
            }
            case Column::Country: {
                return row["countrycode"].toString();
            }
            case Column::PaymentMethod: {
                auto paymentMethodCode = row["online_provider"].toString();
                if (paymentMethodCode == "NATIONAL_BANK") {
                    return QString("National bank transfer");
                }
                return m_paymentMethodNames.value(paymentMethodCode, paymentMethodCode);
            }
            case Column::PaymentMethodDetail: {
                auto paymentMethodDetailText = row["payment_method_detail"].toString();
                QString filteredString; // We can't display emojis in QTreeView
                for (const auto &Char : paymentMethodDetailText) {
                    if (Char.unicode() < 256)
                        filteredString.append(Char);
                    else
                        filteredString.append(" ");
                }

                return filteredString.trimmed();
            }
            case Column::PriceXMR: {
                return QString("%1 %2").arg(row["temp_price"].toString(), row["currency"].toString());
            }
            case Column::Limits: {
                auto minAmount = row["min_amount"].toString();
                auto maxAmount = row["max_amount"].toString();
                if (maxAmount.isEmpty()) {
                    maxAmount = row["max_amount_available"].toString();
                }
                auto currency  = row["currency"].toString();

                if (minAmount.isEmpty() && maxAmount.isEmpty()) {
                    return QString("Up to any amount %1").arg(currency);
                }

                if (!minAmount.isEmpty() && maxAmount.isEmpty()) {
                    return QString("%1 - any amount %2").arg(minAmount, currency);
                }

                if (!minAmount.isEmpty() && !maxAmount.isEmpty()) {
                    return QString("%1 - %2 %3").arg(minAmount, maxAmount, currency);
                }

                if (minAmount.isEmpty() && !maxAmount.isEmpty()) {
                    return QString("Up to %1 %2").arg(maxAmount, currency);
                }

                return QVariant();
            }
        }
    }
    else if (role == Qt::ForegroundRole) {
        switch (col) {
            case Column::PriceXMR: {
                return QVariant(QColor("#388538"));
            }
        }
    }
    else if (role == Qt::FontRole) {
        switch (col) {
            case Column::PriceXMR: {
                auto bigFont = Utils::relativeFont(2);
                bigFont.setBold(true);
                return bigFont;
            }
        }
    }

    return QVariant();
}

void LocalMoneroModel::setData(const QJsonArray &data) {
    beginResetModel();
    m_data = data;
    endResetModel();
}

void LocalMoneroModel::addData(const QJsonArray &data) {
    beginResetModel();

    for (const auto &row : data) {
        m_data.append(row);
    }

    endResetModel();
}

void LocalMoneroModel::clearData() {
    beginResetModel();
    m_data = {};
    endResetModel();
}

void LocalMoneroModel::setPaymentMethods(const QJsonObject &data) {
    beginResetModel();

    m_paymentMethods = data;
    m_paymentMethodNames.clear();
    for (const auto &payment_method : data) {
        auto code = payment_method["code"].toString();
        auto name = payment_method["name"].toString();

        if (!code.isEmpty() && !name.isEmpty()) {
            m_paymentMethodNames[code] = name;
        }
    }

    endResetModel();
}

QJsonObject LocalMoneroModel::getOffer(int index) const {
    return m_data.at(index).toObject();
}
