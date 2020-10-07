// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#include "historywidget.h"
#include "ui_historywidget.h"
#include "dialog/transactioninfodialog.h"
#include "libwalletqt/TransactionHistory.h"
#include "model/TransactionHistoryProxyModel.h"

#include <QIcon>

HistoryWidget::HistoryWidget(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::HistoryWidget)
        , m_contextMenu(new QMenu(this))
        , m_copyMenu(new QMenu("Copy", this))
{
    ui->setupUi(this);
    m_contextMenu->addMenu(m_copyMenu);
    m_contextMenu->addAction(QIcon(":/assets/images/info.png"), "Show details", this, &HistoryWidget::showTxDetails);
    m_contextMenu->addAction(QIcon(":/assets/images/network.png"), "View on block explorer", this, &HistoryWidget::onViewOnBlockExplorer);

    // copy menu
    m_copyMenu->setIcon(QIcon(":/assets/images/copy.png"));
    m_copyMenu->addAction("Transaction ID", this, [this]{copy(copyField::TxID);});
    m_copyMenu->addAction("Date", this, [this]{copy(copyField::Date);});
    m_copyMenu->addAction("Amount", this, [this]{copy(copyField::Amount);});
    m_copyMenu->addAction("Spend proof", this, &HistoryWidget::getSpendProof);

    ui->history->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->history, &QTreeView::customContextMenuRequested, [=](const QPoint & point){
        QModelIndex index = ui->history->indexAt(point);
        if (index.isValid()) {
            m_contextMenu->exec(ui->history->viewport()->mapToGlobal(point));
        }
    });
    connect(ui->search, &QLineEdit::textChanged, this, &HistoryWidget::setSearchFilter);
}

void HistoryWidget::setModel(Coins *coins, TransactionHistoryProxyModel *model, TransactionHistory *txHistory)
{
    m_coins = coins;
    m_model = model;
    m_txHistory = txHistory;
    ui->history->setModel(m_model);

    ui->history->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->history->header()->setSectionResizeMode(TransactionHistoryModel::Description, QHeaderView::Stretch);
    ui->history->hideColumn(TransactionHistoryModel::TxID);
}

void HistoryWidget::showTxDetails() {
    QModelIndex index = ui->history->currentIndex();

    TransactionInfo *i = nullptr;
    m_txHistory->transaction(m_model->mapToSource(index).row(), [&i](TransactionInfo &tInfo) {
        i = &tInfo;
    });

    if (i != nullptr) {
        auto * dialog = new TransactionInfoDialog(m_coins, i, this);
        dialog->exec();
    }
}

void HistoryWidget::onViewOnBlockExplorer() {
    QModelIndex index = ui->history->currentIndex();

    QString txid;
    m_txHistory->transaction(m_model->mapToSource(index).row(), [&txid](TransactionInfo &tInfo) {
        txid = tInfo.hash();
    });
    emit viewOnBlockExplorer(txid);
}

void HistoryWidget::getSpendProof() {
    QModelIndex index = ui->history->currentIndex();

    m_txHistory->transaction(m_model->mapToSource(index).row(), [this](TransactionInfo &tInfo) {
        emit spendProof(tInfo.hash());
    });
}

void HistoryWidget::setSearchText(const QString &text) {
    ui->search->setText(text);
}

void HistoryWidget::setSearchFilter(const QString &filter) {
    if(!m_model) return;
    m_model->setSearchFilter(filter);
}

void HistoryWidget::copy(copyField field) {
    QModelIndex index = ui->history->currentIndex();

    QString data;
    m_txHistory->transaction(m_model->mapToSource(index).row(), [field, &data](TransactionInfo &tInfo) {
        switch(field) {
            case copyField::TxID:
                data = tInfo.hash();
                break;
            case copyField::Date:
                data = tInfo.timestamp().toString("yyyy-MM-dd HH:mm");
                break;
            case copyField::Amount:
                data = tInfo.displayAmount();
                break;
        }
    });

    Utils::copyToClipboard(data);
}

HistoryWidget::~HistoryWidget() {
    delete ui;
}
