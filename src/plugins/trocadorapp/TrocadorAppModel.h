// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_TROCADORAPPMODEL_H
#define FEATHER_TROCADORAPPMODEL_H

#include <QAbstractTableModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

class TrocadorAppModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column
    {
        Exchange,
        Rate,
        Spread,
        KYC,
        COUNT
    };

    TrocadorAppModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setData(const QJsonArray &data);
    void addData(const QJsonArray &data);
    void clearData();

    void addTradeId(const QString &tradeId);
    QString getTradeId() const;

    QJsonObject getOffer(int index) const;

private:
    QJsonArray m_data;
    QString m_tradeId;
};

#endif //FEATHER_TROCADORAPPMODEL_H
