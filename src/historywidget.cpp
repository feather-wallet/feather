// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "historywidget.h"
#include "ui_historywidget.h"
#include "dialog/transactioninfodialog.h"
#include "dialog/TxProofDialog.h"
#include <QMessageBox>

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
    m_copyMenu->addAction("Description", this, [this]{copy(copyField::Description);});
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

    connect(ui->btn_moreInfo, &QPushButton::clicked, this, &HistoryWidget::showSyncNoticeMsg);
    connect(ui->btn_close, &QPushButton::clicked, [this]{
        config()->set(Config::showHistorySyncNotice, false);
        ui->syncNotice->hide();
    });
}

void HistoryWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->history->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);

    auto *tx = ui->history->currentEntry();
    if (!tx) return;

    bool unconfirmed = tx->isFailed() || tx->isPending();
    if (AppContext::txCache.contains(tx->hash()) && unconfirmed && tx->direction() != TransactionInfo::Direction_In) {
        menu.addAction(QIcon(":/assets/images/info.png"), "Resend transaction", this, &HistoryWidget::onResendTransaction);
    }

    menu.addMenu(m_copyMenu);
    menu.addAction(QIcon(":/assets/images/info.png"), "Show details", this, &HistoryWidget::showTxDetails);
    menu.addAction(QIcon(":/assets/images/network.png"), "View on block explorer", this, &HistoryWidget::onViewOnBlockExplorer);
    menu.addAction("Create tx proof", this, &HistoryWidget::createTxProof);

    menu.exec(ui->history->viewport()->mapToGlobal(point));
}

void HistoryWidget::onResendTransaction() {
    auto *tx = ui->history->currentEntry();
    if (tx) {
        QString txid = tx->hash();
        emit resendTransaction(txid);
    }
}

void HistoryWidget::setModel(TransactionHistoryProxyModel *model, Wallet *wallet)
{
    m_model = model;
    m_wallet = wallet;
    m_txHistory = m_wallet->history();
    ui->history->setHistoryModel(m_model);
    m_wallet->transactionHistoryModel()->amountPrecision = config()->get(Config::amountPrecision).toInt();

    // Load view state
    QByteArray historyViewState = QByteArray::fromBase64(config()->get(Config::GUI_HistoryViewState).toByteArray());

    if (!historyViewState.isEmpty()) {
      ui->history->setViewState(historyViewState);
    }
}

void HistoryWidget::resetModel()
{
    // Save view state
    config()->set(Config::GUI_HistoryViewState, ui->history->viewState().toBase64());
    config()->sync();

    ui->history->setModel(nullptr);
}

void HistoryWidget::showTxDetails() {
    auto *tx = ui->history->currentEntry();
    if (!tx) return;

    auto *dialog = new TransactionInfoDialog(m_wallet, tx, this);
    connect(dialog, &TransactionInfoDialog::resendTranscation, [this](const QString &txid){
       emit resendTransaction(txid);
    });
    dialog->show();
}

void HistoryWidget::onViewOnBlockExplorer() {
    auto *tx = ui->history->currentEntry();
    if (!tx) return;

    QString txid = tx->hash();
    emit viewOnBlockExplorer(txid);
}

void HistoryWidget::setSearchText(const QString &text) {
    ui->search->setText(text);
}

void HistoryWidget::setSearchFilter(const QString &filter) {
    if (!m_model) return;
    m_model->setSearchFilter(filter);
    ui->history->setSearchMode(!filter.isEmpty());
}

void HistoryWidget::createTxProof() {
    auto *tx = ui->history->currentEntry();
    if (!tx) return;

    auto *dialog = new TxProofDialog(this, m_wallet, tx);
    dialog->exec();
    dialog->deleteLater();
}

void HistoryWidget::copy(copyField field) {
    auto *tx = ui->history->currentEntry();
    if (!tx) return;

    QString data = [field, tx]{
        switch(field) {
            case copyField::TxID:
                return tx->hash();
            case copyField::Date:
                return tx->timestamp().toString("yyyy-MM-dd HH:mm");
            case copyField::Amount:
                return tx->displayAmount();
            default:
                return QString("");
        }
    }();

    Utils::copyToClipboard(data);
}

void HistoryWidget::onWalletOpened() {
    ui->syncNotice->setVisible(config()->get(Config::showHistorySyncNotice).toBool());
}

void HistoryWidget::onWalletRefreshed() {
    ui->syncNotice->hide();
}

void HistoryWidget::showSyncNoticeMsg() {
    QMessageBox::information(this, "Sync notice",
                             "The wallet needs to scan the blockchain to find your transactions. "
                             "The status bar will show you how many blocks are still remaining.\n"
                             "\n"
                             "The history page will update once synchronization has finished. "
                             "To update the history page during synchronization press Ctrl+R.");
}

HistoryWidget::~HistoryWidget() {
    delete ui;
}
