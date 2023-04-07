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
    const auto row = m_data.at(index.row()).toObject();

    if (row.isEmpty()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        switch (col) {
            case Column::Exchange: {
                return row["provider"].toString();
            }
            case Column::Rate: {
                return row["insurance"].toInt();
            }
            case Column::Spread: {
                return row["waste"].toString();
            }
            case Column::KYC: {
                return row["kycrating"].toString();
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

QJsonObject TrocadorAppModel::getOffer(int index) const {
    return m_data.at(index).toObject();
}
