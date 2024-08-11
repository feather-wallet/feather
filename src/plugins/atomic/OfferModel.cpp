//
// Created by dev on 5/23/24.
//

#include "OfferModel.h"


OfferModel::OfferModel(QObject *parent)
        : QAbstractTableModel(parent)
{

}

void OfferModel::clear() {
    beginResetModel();

    m_offers.clear();

    endResetModel();
}

void OfferModel::updateOffers(const QList<QSharedPointer<OfferEntry>> &posts) {
    beginResetModel();
    m_offers.clear();
    for (const auto& post : posts) {
        m_offers.push_back(post);
    }

    endResetModel();
}

int OfferModel::rowCount(const QModelIndex &parent) const{
    if (parent.isValid()) {
        return 0;
    }
    return m_offers.count();
}

int OfferModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return OfferModel::ModelColumn::COUNT;
}

QVariant OfferModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_offers.count())
        return {};

    QSharedPointer<OfferEntry> post = m_offers.at(index.row());

    if(role == Qt::DisplayRole || role == Qt::UserRole) {
        switch(index.column()) {
            case Price: {
                if (role == Qt::UserRole) {
                    return post->price;
                }
                return QString::number(post->price);
            }
            case Address:
                return post->address;
            case Min_Amount:{
                if (role == Qt::UserRole) {
                    return post->min;
                }
                return QString::number(post->min);
            }
            case Max_Amount: {
                if (role == Qt::UserRole) {
                    return post->max;
                }
                return QString::number(post->max);
            }
            default:
                return {};
        }
    }
    else if (role == Qt::TextAlignmentRole) {
        switch(index.column()) {
            case Price:
            case Min_Amount:
            case Max_Amount:
                return Qt::AlignRight;
            default:
                return {};
        }
    }
    return {};
}

QVariant OfferModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal)
    {
        switch(section) {
            case Price:
                return QString("Price (BTC) ");
            case Min_Amount:
                return QString("Min Amount (BTC)");
            case Max_Amount:
                return QString("Max Amount (BTC)");
            case Address:
                return QString("P2P Vendor Address");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QSharedPointer<OfferEntry> OfferModel::post(int row) {
    if (row < 0 || row >= m_offers.size()) {
        qCritical("%s: no reddit post for index %d", __FUNCTION__, row);
        return QSharedPointer<OfferEntry>();
    }

    return m_offers.at(row);
}