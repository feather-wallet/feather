// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

#include "SubaddressAccountModel.h"
#include "SubaddressAccount.h"
#include <wallet/api/wallet2_api.h>

#include <QDebug>
#include <QHash>

SubaddressAccountModel::SubaddressAccountModel(QObject *parent, SubaddressAccount *subaddressAccount)
    : QAbstractListModel(parent), m_subaddressAccount(subaddressAccount)
{
    connect(m_subaddressAccount, &SubaddressAccount::refreshStarted, this, &SubaddressAccountModel::startReset);
    connect(m_subaddressAccount, &SubaddressAccount::refreshFinished, this, &SubaddressAccountModel::endReset);
}

void SubaddressAccountModel::startReset(){
    beginResetModel();
}
void SubaddressAccountModel::endReset(){
    endResetModel();
}

int SubaddressAccountModel::rowCount(const QModelIndex &) const
{
    return m_subaddressAccount->count();
}

QVariant SubaddressAccountModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_subaddressAccount->count())
        return {};

    QVariant result;

    bool found = m_subaddressAccount->getRow(index.row(), [&result, &role](const Monero::SubaddressAccountRow &row) {
        switch (role) {
        case SubaddressAccountAddressRole:
            result = QString::fromStdString(row.getAddress());
            break;
        case SubaddressAccountLabelRole:
            result = QString::fromStdString(row.getLabel());
            break;
        case SubaddressAccountBalanceRole:
            result = QString::fromStdString(row.getBalance());
            break;
        case SubaddressAccountUnlockedBalanceRole:
            result = QString::fromStdString(row.getUnlockedBalance());
            break;
        default:
            qCritical() << "Unimplemented role" << role;
        }
    });
    if (!found) {
        qCritical("%s: internal error: invalid index %d", __FUNCTION__, index.row());
    }

    return result;
}

QHash<int, QByteArray> SubaddressAccountModel::roleNames() const
{
    static QHash<int, QByteArray> roleNames;
    if (roleNames.empty())
    {
        roleNames.insert(SubaddressAccountAddressRole, "address");
        roleNames.insert(SubaddressAccountLabelRole, "label");
        roleNames.insert(SubaddressAccountBalanceRole, "balance");
        roleNames.insert(SubaddressAccountUnlockedBalanceRole, "unlockedBalance");
    }
    return roleNames;
}
