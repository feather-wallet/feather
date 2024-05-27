//
// Created by dev on 5/23/24.
//

#ifndef FEATHER_OFFERMODEL_H
#define FEATHER_OFFERMODEL_H
#include <QAbstractTableModel>

#include "Offer.h"

class OfferModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum ModelColumn
    {
        Price = 0,
        Min_Amount,
        Max_Amount,
        Address,
        COUNT
    };

    explicit OfferModel(QObject *parent);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    void clear();
    void updateOffers(const QList<QSharedPointer<OfferEntry>>& posts);

    QSharedPointer<OfferEntry> post(int row);

private:
    QList<QSharedPointer<OfferEntry>> m_offers;
};


#endif //FEATHER_OFFERMODEL_H
