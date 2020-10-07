// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_SUBADDRESSPROXYMODEL_H
#define FEATHER_SUBADDRESSPROXYMODEL_H

#include "libwalletqt/Subaddress.h"

#include <QSortFilterProxyModel>

class SubaddressProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SubaddressProxyModel(QObject* parent, Subaddress *subaddress, bool hidePrimary = true);
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const;

public slots:
    void setSearchFilter(const QString& searchString){
        m_searchRegExp.setPattern(searchString);
        m_searchCaseSensitiveRegExp.setPattern(searchString);
        invalidateFilter();
    }
    void setShowUsed(const bool showUsed){
        m_showUsed = showUsed;
        invalidateFilter();
    }
private:
    Subaddress *m_subaddress;

    QRegExp m_searchRegExp;
    QRegExp m_searchCaseSensitiveRegExp;
    bool m_showUsed = false;
    bool m_hidePrimary;
};

#endif //FEATHER_SUBADDRESSPROXYMODEL_H
