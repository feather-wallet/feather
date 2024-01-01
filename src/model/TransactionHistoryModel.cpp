// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "TransactionHistoryModel.h"
#include "TransactionHistory.h"
#include "constants.h"
#include "utils/config.h"
#include "utils/ColorScheme.h"
#include "utils/Icons.h"
#include "utils/AppData.h"
#include "utils/Utils.h"
#include "libwalletqt/rows/TransactionRow.h"

TransactionHistoryModel::TransactionHistoryModel(QObject *parent)
    : QAbstractTableModel(parent),
    m_transactionHistory(nullptr)
{
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

TransactionRow* TransactionHistoryModel::entryFromIndex(const QModelIndex &index) const {
    Q_ASSERT(index.isValid() && index.row() < m_transactionHistory->count());
    return m_transactionHistory->transaction(index.row());
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

    bool found = m_transactionHistory->transaction(index.row(), [this, &index, &result, &role](const TransactionRow &tInfo) {
        if(role == Qt::DisplayRole || role == Qt::EditRole || role == Qt::UserRole) {
            result = parseTransactionInfo(tInfo, index.column(), role);
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
                        result = QVariant(icons()->icon("warning.png"));
                    else if (tInfo.isPending())
                        result = QVariant(icons()->icon("unconfirmed.png"));
                    else if (tInfo.confirmations() <= (1.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(icons()->icon("clock1.png"));
                    else if (tInfo.confirmations() <= (2.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(icons()->icon("clock2.png"));
                    else if (tInfo.confirmations() <= (3.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(icons()->icon("clock3.png"));
                    else if (tInfo.confirmations() <= (4.0/5.0 * tInfo.confirmationsRequired()))
                        result = QVariant(icons()->icon("clock4.png"));
                    else if (tInfo.confirmations() < tInfo.confirmationsRequired())
                        result = QVariant(icons()->icon("clock5.png"));
                    else if (tInfo.confirmations())
                        result = QVariant(icons()->icon("confirmed.svg"));
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
                    if (tInfo.balanceDelta() < 0) {
                        result = QVariant(QColor("#BC1E1E"));
                    }
                }
            }
        }
        else if (role == Qt::FontRole) {
            switch(index.column()) {
                case Column::TxID:
                {
                    result = Utils::getMonospaceFont();
                }
            }
        }
    });

    if (!found) {
        qCritical("%s: internal error: no transaction info for index %d", __FUNCTION__, index.row());
    }
    return result;
}

QVariant TransactionHistoryModel::parseTransactionInfo(const TransactionRow &tInfo, int column, int role) const
{
    switch (column)
    {
        case Column::Date:
        {
            if (role == Qt::UserRole) {
                return tInfo.timestamp();
            }
            return tInfo.timestamp().toString(QString("%1 %2 ").arg(conf()->get(Config::dateFormat).toString(),
                                                                    conf()->get(Config::timeFormat).toString()));
        }
        case Column::Description:
            return tInfo.description();
        case Column::Amount:
        {
            if (role == Qt::UserRole) {
                return tInfo.balanceDelta();
            }
            QString amount = QString::number(tInfo.balanceDelta() / constants::cdiv, 'f', conf()->get(Config::amountPrecision).toInt());
            amount = (tInfo.balanceDelta() < 0) ? amount : "+" + amount;
            return amount;
        }
        case Column::TxID: {
            if (conf()->get(Config::historyShowFullTxid).toBool()) {
                return tInfo.hash();
            }
            return Utils::displayAddress(tInfo.hash(), 1);
        }
        case Column::FiatAmount:
        {
            double usd_price = appData()->txFiatHistory->get(tInfo.timestamp().toString("yyyyMMdd"));
            if (usd_price == 0.0) {
                return QString("?");
            }

            double usd_amount = usd_price * (abs(tInfo.balanceDelta()) / constants::cdiv);

            QString preferredFiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
            if (preferredFiatCurrency != "USD") {
                usd_amount = appData()->prices.convert("USD", preferredFiatCurrency, usd_amount);
            }
            if (role == Qt::UserRole) {
                return usd_amount;
            }
            if (usd_amount == 0.0) {
                return QString("?");
            }

            double fiat_rounded = ceil(Utils::roundSignificant(usd_amount, 3) * 100.0) / 100.0;
            return QString("%1").arg(Utils::amountToCurrencyString(fiat_rounded, preferredFiatCurrency));
        }
        default:
        {
            qCritical() << "Unimplemented role";
            return {};
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
                m_transactionHistory->transaction(index.row(), [this, &hash, &value](const TransactionRow &tInfo){
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
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    if (index.column() == Description)
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;

    return QAbstractTableModel::flags(index);
}
