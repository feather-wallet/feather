// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SUBADDRESSPROXYMODEL_H
#define FEATHER_SUBADDRESSPROXYMODEL_H

#include "libwalletqt/Subaddress.h"

#include <QSortFilterProxyModel>

class SubaddressProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit SubaddressProxyModel(QObject* parent, Subaddress *subaddress);
    [[nodiscard]] bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

public slots:
    void setSearchFilter(const QString& searchString){
        m_searchRegExp.setPattern(searchString);
        m_searchCaseSensitiveRegExp.setPattern(searchString);
        invalidateFilter();
    }

private:
    Subaddress *m_subaddress;
    QRegularExpression m_searchRegExp;
    QRegularExpression m_searchCaseSensitiveRegExp;
};

#endif //FEATHER_SUBADDRESSPROXYMODEL_H
