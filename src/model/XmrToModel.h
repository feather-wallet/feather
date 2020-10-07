// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_XMRTOMODEL_H
#define FEATHER_XMRTOMODEL_H

#include <QAbstractTableModel>

class XmrToOrder;
class XmrToModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum ModelColumn
    {
        Status = 0,
        ID,
        Conversion,
        Rate,
        Destination,
        ErrorMsg,
        COUNT
    };

    XmrToModel(QList<XmrToOrder*> *orders, QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QList<XmrToOrder*> *orders;

public slots:
    void update();
};


#endif //FEATHER_XMRTOMODEL_H
