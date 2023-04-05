// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "TrocadorAppModel.h"
#include <QColor>
#include "utils/Utils.h"

TrocadorAppModel::TrocadorAppModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

int TrocadorAppModel::rowCount(const QModelIndex &parent) const {
    return m_data.count();
}

int TrocadorAppModel::columnCount(const QModelIndex &parent) const {
    return Column::COUNT;
}

QVariant TrocadorAppModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Exchange:
                return QString("Exchange");
            case Rate:
                return QString("Rate");
            case Spread:
                return QString("Spread");
            case KYC:
                return QString("KYC");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant TrocadorAppModel::data(const QModelIndex &index, int role) const {
    const int col = index.column();
    const auto row = m_data.at(index.row()).toObject()["data"].toObject();

    if (row.isEmpty()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (col) {
            case Column::Exchange: {
                auto exchange = row["profile"].toObject();
                return exchange["name"].toString();
                // TODO: online indicator
            }
            case Column::Rate: {
                auto paymentMethodCode = row["online_provider"].toString();
                if (paymentMethodCode == "NATIONAL_BANK") {
                    return QString("National bank transfer");
                }
                return m_spread.value(paymentMethodCode, paymentMethodCode);
            }
            case Column::Spread: {
                auto spreadText = row["spread"].toString();
                QString filteredString; // We can't display emojis in QTreeView
                for (const auto &Char : spreadText) {
                    if (Char.unicode() < 256)
                        filteredString.append(Char);
                    else
                        filteredString.append(" ");
                }

                return filteredString.trimmed();
            }
            case Column::KYC: {
                return QString("%1 %2").arg(row["temp_kyc"].toString(), row["currency"].toString());
            }
        }
    }
    else if (role == Qt::ForegroundRole) {
        switch (col) {
            case Column::KYC: {
                return QVariant(QColor("#388538"));
            }
        }
    }
    else if (role == Qt::FontRole) {
        switch (col) {
            case Column::KYC: {
                auto bigFont = Utils::relativeFont(2);
                bigFont.setBold(true);
                return bigFont;
            }
        }
    }

    return QVariant();
}

void TrocadorAppModel::setData(const QJsonArray &data) {
    beginResetModel();
    m_data = data;
    endResetModel();
}

void TrocadorAppModel::addData(const QJsonArray &data) {
    beginResetModel();

    for (const auto &row : data) {
        m_data.append(row);
    }

    endResetModel();
}

void TrocadorAppModel::clearData() {
    beginResetModel();
    m_data = {};
    endResetModel();
}

void TrocadorAppModel::setPaymentMethods(const QJsonObject &data) {
    beginResetModel();

    m_rate = data;
    m_spread.clear();
    for (const auto &payment_method : data) {
        auto code = payment_method.toObject()["code"].toString();
        auto name = payment_method.toObject()["name"].toString();

        if (!code.isEmpty() && !name.isEmpty()) {
            m_spread[code] = name;
        }
    }

    endResetModel();
}

QJsonObject TrocadorAppModel::getOffer(int index) const {
    return m_data.at(index).toObject();
}
