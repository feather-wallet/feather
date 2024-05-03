// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "TxPoolViewerDialog.h"
#include "ui_TxPoolViewerDialog.h"

#include <QTreeWidgetItem>

#include "utils/Utils.h"
#include "utils/ColorScheme.h"
#include "libwalletqt/WalletManager.h"

TxPoolViewerDialog::TxPoolViewerDialog(QWidget *parent, Wallet *wallet)
        : QDialog(parent)
        , ui(new Ui::TxPoolViewerDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_refresh, &QPushButton::clicked, this, &TxPoolViewerDialog::refresh);
    connect(m_wallet, &Wallet::poolStats, this, &TxPoolViewerDialog::onTxPoolBacklog);

    ui->tree_pool->sortByColumn(2, Qt::DescendingOrder);

    this->refresh();
}

void TxPoolViewerDialog::refresh() {
    ui->btn_refresh->setEnabled(false);
    m_wallet->getTxPoolStatsAsync();
}

class TxPoolSortItem : public QTreeWidgetItem {
public:
    using QTreeWidgetItem::QTreeWidgetItem;

    bool operator<(const QTreeWidgetItem &other) const override {
        int column = treeWidget()->sortColumn();

        return this->data(column, Qt::UserRole).toUInt() < other.data(column, Qt::UserRole).toUInt();
    }
};

void TxPoolViewerDialog::onTxPoolBacklog(const QVector<TxBacklogEntry> &txPool, const QVector<quint64> &baseFees, quint64 blockWeightLimit) {
    ui->btn_refresh->setEnabled(true);

    if (baseFees.size() != 4) {
        return;
    }

    ui->tree_pool->clear();
    ui->tree_feeTiers->clear();

    m_feeTierStats.clear();
    for (int i = 0; i < 4; i++) {
        m_feeTierStats.push_back(FeeTierStats{});
    }

    ui->label_transactions->setText(QString::number(txPool.size()));

    uint64_t totalWeight = 0;
    uint64_t totalFees = 0;
    for (const auto &entry : txPool) {
        totalWeight += entry.weight;
        totalFees += entry.fee;

        auto* item = new TxPoolSortItem();
        item->setText(0, QString("%1 B").arg(QString::number(entry.weight)));
        item->setData(0, Qt::UserRole, entry.weight);
        item->setTextAlignment(0, Qt::AlignRight);

        item->setText(1, QString("%1 XMR").arg(WalletManager::displayAmount(entry.fee)));
        item->setData(1, Qt::UserRole, entry.fee);
        item->setTextAlignment(1, Qt::AlignRight);

        quint64 fee_per_byte = entry.fee / entry.weight;
        item->setText(2, QString::number(entry.fee / entry.weight));
        item->setData(2, Qt::UserRole, entry.fee / entry.weight);
        item->setTextAlignment(2, Qt::AlignRight);

        if (fee_per_byte == baseFees[0]) {
            item->setBackground(2, QBrush(ColorScheme::BLUE.asColor(true)));
        }
        if (fee_per_byte == baseFees[1]) {
            item->setBackground(2, QBrush(ColorScheme::GREEN.asColor(true)));
        }
        if (fee_per_byte == baseFees[2]) {
            item->setBackground(2, QBrush(ColorScheme::YELLOW.asColor(true)));
        }
        if (fee_per_byte == baseFees[3]) {
            item->setBackground(2, QBrush(ColorScheme::RED.asColor(true)));
        }

        if (fee_per_byte >= baseFees[3]) {
            m_feeTierStats[3].weightFromTip += entry.weight;
        }
        if (fee_per_byte >= baseFees[2]) {
            m_feeTierStats[2].weightFromTip += entry.weight;
        }
        if (fee_per_byte >= baseFees[1]) {
            m_feeTierStats[1].weightFromTip += entry.weight;
        }
        if (fee_per_byte >= baseFees[0]) {
            m_feeTierStats[0].weightFromTip += entry.weight;
        }

        ui->tree_pool->addTopLevelItem(item);
    }

    ui->tree_pool->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tree_pool->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    ui->label_totalWeight->setText(Utils::formatBytes(totalWeight));
    ui->label_totalFees->setText(QString("%1 XMR").arg(WalletManager::displayAmount(totalFees)));

    quint64 fullRewardZone = blockWeightLimit >> 1;
    ui->label_blockWeightLimit->setText(Utils::formatBytes(fullRewardZone));

    for (int i = 0; i < 4; i++) {
        QString tierName;
        switch (i) {
            case 0:
                tierName = "Low";
                break;
            case 1:
                tierName = "Normal";
                break;
            case 2:
                tierName = "High";
                break;
            case 3:
            default:
                tierName = "Highest ";
                break;
        }

        auto* item = new QTreeWidgetItem();
        item->setText(0, tierName);

        item->setText(1, QString::number(baseFees[i]));
        item->setTextAlignment(1, Qt::AlignRight);

        item->setText(2, QString(" %1 blocks").arg(QString::number(m_feeTierStats[i].weightFromTip / fullRewardZone))); // approximation
        item->setTextAlignment(2, Qt::AlignRight);

        item->setText(3, QString("%1 kB").arg(QString::number(m_feeTierStats[i].weightFromTip / 1000)));
        item->setTextAlignment(3, Qt::AlignRight);

        ui->tree_feeTiers->addTopLevelItem(item);
    }

    ui->tree_feeTiers->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tree_feeTiers->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tree_feeTiers->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tree_feeTiers->headerItem()->setTextAlignment(2, Qt::AlignRight);
    ui->tree_feeTiers->headerItem()->setTextAlignment(3, Qt::AlignRight);
}

TxPoolViewerDialog::~TxPoolViewerDialog() = default;
