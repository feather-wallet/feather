// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "TransactionHistoryModel.h"
#include "TransactionHistory.h"
#include "TransactionInfo.h"
#include "globals.h"

TransactionHistoryModel::TransactionHistoryModel(QObject *parent)
    : QAbstractTableModel(parent),
    m_transactionHistory(nullptr)
{
    m_unconfirmedTx = QIcon(":/assets/images/unconfirmed.png");
    m_warning = QIcon(":/assets/images/warning.png");
    m_clock1 = QIcon(":/assets/images/clock1.png");
    m_clock2 = QIcon(":/assets/images/clock2.png");
    m_clock3 = QIcon(":/assets/images/clock3.png");
    m_clock4 = QIcon(":/assets/images/clock4.png");
    m_clock5 = QIcon(":/assets/images/clock5.png");
    m_confirmedTx = QIcon(":/assets/images/confirmed.png");
}

void TransactionHistoryModel::setTransactionHistory(TransactionHistory *th) {
    beginResetModel();
    m_transactionHistory = th;
    endResetModel();

    connect(m_transactionHistory, &TransactionHistory::refreshStarted,
            this, &TransactionHistoryModel::beginResetModel);
    connect(m_transactionHistory, &TransactionHistory::refreshFinished,
            this, &TransactionHistoryModel::endResetModel);

    emit transactionHistoryChanged();
}

TransactionHistory *TransactionHistoryModel::transactionHistory() const {
    return m_transactionHistory;
}

int TransactionHistoryModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    } else {
        return m_transactionHistory ? m_transactionHistory->count() : 0;
    }
}

int TransactionHistoryModel::columnCount(const QModelIndex &parent) const {
    if (parent.isValid()) {
        return 0;
    }

    return Column::COUNT;
}

QVariant TransactionHistoryModel::data(const QModelIndex &index, int role) const {
    if (!m_transactionHistory) {
        return QVariant();
    }

    if (!index.isValid() || index.row() < 0 || static_cast<quint64>(index.row()) >= m_transactionHistory->count())
        return QVariant();

    QVariant result;

    bool found = m_transactionHistory->transaction(index.row(), [this, &index, &result, &role](const TransactionInfo &tInfo) {
        if(role == Qt::DisplayRole || role == Qt::EditRole) {
            result = parseTransactionInfo(tInfo, index.column());
        }
        else if (role == Qt::TextAlignmentRole) {
            switch (index.column()) {
                case Column::Amount:
                case Column::FiatAmount:
                    result = Qt::AlignRight;
            }
        }
        else if (role == Qt::DecorationRole) {
            switch (index.column()) {
                case Column::Date:
                {
                    if (tInfo.isFailed())
                        result = QVariant(m_warning);
                    else if (tInfo.isPending())
                        result = QVariant(m_unconfirmedTx);
                    else if (tInfo.confirmations() <= (1.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(m_clock1);
                    else if (tInfo.confirmations() <= (2.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(m_clock2);
                    else if (tInfo.confirmations() <= (3.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(m_clock3);
                    else if (tInfo.confirmations() <= (4.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(m_clock4);
                    else if (tInfo.confirmations() < tInfo.confirmationsRequired())
                        result = QVariant(m_clock5);
                    else if (tInfo.confirmations())
                        result = QVariant(m_confirmedTx);
                }
            }
        }
        else if (role == Qt::ToolTipRole) {
            switch(index.column()) {
                case Column::Date:
                {
                    if (tInfo.isFailed())
                        result = "Transaction failed";
                    else if (tInfo.confirmations() < tInfo.confirmationsRequired())
                        result = QString("%1/%2 confirmations").arg(QString::number(tInfo.confirmations()), QString::number(tInfo.confirmationsRequired()));
                    else
                        result = QString("%1 confirmations").arg(QString::number(tInfo.confirmations()));
                }
            }
        }
        else if (role == Qt::ForegroundRole) {
            switch(index.column()) {
                case Column::FiatAmount:
                case Column::Amount:
                {
                    if (tInfo.direction() == TransactionInfo::Direction_Out) {
                        result = QVariant(QColor("#BC1E1E"));
                    }
                }
            }
        }
    });

    if (!found) {
        qCritical("%s: internal error: no transaction info for index %d", __FUNCTION__, index.row());
    }
    return result;
}

QVariant TransactionHistoryModel::parseTransactionInfo(const TransactionInfo &tInfo, int column) const
{
    switch (column)
    {
        case Column::Date:
            return tInfo.timestamp().toString("yyyy-MM-dd HH:mm");
        case Column::Description: {
            // if this tx is still in the pool, then we wont get the
            // description. We've cached it inside `AppContext::txDescriptionCache`
            // for the time being.
            if(tInfo.isPending()) {
                auto hash = tInfo.hash();
                if (AppContext::txDescriptionCache.contains(hash))
                    return AppContext::txDescriptionCache[hash];
            }
            return tInfo.description();
        }
        case Column::Amount:
        {
            QString amount = QString::number(tInfo.balanceDelta() / globals::cdiv, 'f', 4);
            amount = (tInfo.direction() == TransactionInfo::Direction_Out) ? "-" + amount : "+" + amount;
            return amount;
        }
        case Column::TxID:
            return tInfo.hash();
        case Column::FiatAmount:
        {
            double usd_price = AppContext::txFiatHistory->get(tInfo.timestamp().toString("yyyyMMdd"));
            if (usd_price == 0.0)
                return QVariant("?");

            double usd_amount = usd_price * (tInfo.balanceDelta() / globals::cdiv);
            if(this->preferredFiatSymbol != "USD")
                usd_amount = AppContext::prices->convert("USD", this->preferredFiatSymbol, usd_amount);
            double fiat_rounded = ceil(Utils::roundSignificant(usd_amount, 3) * 100.0) / 100.0;

            return QString("%1").arg(Utils::amountToCurrencyString(fiat_rounded, this->preferredFiatSymbol));
        }
        default:
        {
            qCritical() << "Unimplemented role";
            return QVariant();
        }
    }
}

QVariant TransactionHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(orientation)
    if (role != Qt::DisplayRole) {
        return QVariant();
    }
    if (orientation == Qt::Horizontal) {
        switch(section) {
            case Column::Date:
                return QString("Date");
            case Column::Description:
                return QString("Description");
            case Column::Amount:
                return QString("Amount");
            case Column::TxID:
                return QString("Txid");
            case Column::FiatAmount:
                return QString("Fiat");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

bool TransactionHistoryModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.isValid() && role == Qt::EditRole) {
        QString hash;

        switch (index.column()) {
            case Column::Description:
            {
                m_transactionHistory->transaction(index.row(), [this, &hash, &value](const TransactionInfo &tInfo){
                    hash = tInfo.hash();
                });
                m_transactionHistory->setTxNote(hash, value.toString());
                break;
            }
            default:
                return false;
        }

        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
        return true;
    }
    return false;
}

Qt::ItemFlags TransactionHistoryModel::flags(const QModelIndex &index) const {
    bool isPending;
    m_transactionHistory->transaction(index.row(), [this, &isPending](const TransactionInfo &tInfo){
        isPending = tInfo.isPending();
    });

    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Description && !isPending)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}
