// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

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

    ui->history->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->history, &QTreeView::customContextMenuRequested, this, &HistoryWidget::showContextMenu);
    connect(ui->search, &QLineEdit::textChanged, this, &HistoryWidget::setSearchFilter);

    connect(ui->history, &QTreeView::doubleClicked, [this](QModelIndex index){
        if (m_model == nullptr) return;
        if (!(m_model->flags(index) & Qt::ItemIsEditable)) {
            this->showTxDetails();
        }
    });

}

void HistoryWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->history->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);
    TransactionInfo::Direction direction;
    QString txid;
    bool unconfirmed;
    m_txHistory->transaction(m_model->mapToSource(index).row(), [&direction, &txid, &unconfirmed](TransactionInfo &tInfo) {
        direction = tInfo.direction();
        txid = tInfo.hash();
        unconfirmed = tInfo.isFailed() || tInfo.isPending();
    });

    if (AppContext::txCache.contains(txid) && unconfirmed && direction != TransactionInfo::Direction_In) {
        menu.addAction(QIcon(":/assets/images/info.png"), "Resend transaction", this, &HistoryWidget::onResendTransaction);
    }

    menu.addMenu(m_copyMenu);
    menu.addAction(QIcon(":/assets/images/info.png"), "Show details", this, &HistoryWidget::showTxDetails);
    menu.addAction(QIcon(":/assets/images/network.png"), "View on block explorer", this, &HistoryWidget::onViewOnBlockExplorer);

    menu.exec(ui->history->viewport()->mapToGlobal(point));
}

void HistoryWidget::onResendTransaction() {
    QModelIndex index = ui->history->currentIndex();
    QString txid;
    m_txHistory->transaction(m_model->mapToSource(index).row(), [&txid](TransactionInfo &tInfo) {
        txid = tInfo.hash();
    });

    emit resendTransaction(txid);
}

void HistoryWidget::setModel(TransactionHistoryProxyModel *model, Wallet *wallet)
{
    m_model = model;
    m_wallet = wallet;
    m_txHistory = m_wallet->history();
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
        auto * dialog = new TransactionInfoDialog(m_wallet, i, this);
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
