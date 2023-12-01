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
            case Amount:
                return QString("Amount");
            case Rate:
                return QString ("Rate");
            case Insurance:
                return QString("Insurance");
            case Spread:
                return QString("Spread");
            case KYCRating:
                return QString("KYC Rating");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QVariant TrocadorAppModel::data(const QModelIndex &index, int role) const {
    const int col = index.column();
    const auto row = m_data.at(index.row()).toObject();

    if (row.isEmpty()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (col) {
            case Column::Exchange: {
                return row["provider"].toString();
            }
            case Column::Amount: {
                if(row.contains("amount_to"))
                    return row["amount_to"].toString();
                else 
                    return row["amount_from"].toString();
            }
            case Column::Rate: {
                if(row["fixed"].toString() == "True")
                    return "Fixed";
                else
                    return "Floating";
            }
            case Column::Insurance: {
                return QString("%1%").arg(row["insurance"].toInt());
            }
            case Column::Spread: {
                return QString("%1%").arg(row["waste"].toString());
            }
            case Column::KYCRating: {
                return row["kycrating"].toString();
            }
        }
    }
    
    else if (role == Qt::ForegroundRole) {
        switch (col) {
            case Column::KYCRating: {
                if (row["kycrating"].toString() == "A")
                    return QVariant(QColor("#008000"));
                else if (row["kycrating"].toString() == "B")
                    return QVariant(QColor("#4ECB4E"));
                else if (row["kycrating"].toString() == "C")
                    return QVariant(QColor("#E9E90E"));
                else if (row["kycrating"].toString() == "D")
                    return QVariant(QColor("#D56242"));
            }
        }
    }

        else if (role == Qt::FontRole) {
        switch (col) {
            case Column::KYCRating: {
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

void TrocadorAppModel::addTradeId(const QString &tradeId) {
    beginResetModel();
    m_tradeId = tradeId;
    endResetModel();
}

QString TrocadorAppModel::getTradeId() const {
    return m_tradeId;
}
