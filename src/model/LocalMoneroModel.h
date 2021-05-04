// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_LOCALMONEROMODEL_H
#define FEATHER_LOCALMONEROMODEL_H

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class LocalMoneroModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        Seller = 0,
        Country,
        PaymentMethod,
        PaymentMethodDetail,
        PriceXMR,
        Limits,
        COUNT
    };

    LocalMoneroModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setData(const QJsonArray &data);
    void setPaymentMethods(const QJsonObject &data);
    void addData(const QJsonArray &data);
    void clearData();

    QJsonObject getOffer(int index) const;

private:
    QJsonArray m_data;
    QJsonObject m_paymentMethods;
    QHash<QString, QString> m_paymentMethodNames;
};

#endif //FEATHER_LOCALMONEROMODEL_H
