// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "HistoryWidget.h"
#include "ui_HistoryWidget.h"

#include <QMessageBox>

#include "dialog/TxInfoDialog.h"
#include "dialog/TxProofDialog.h"
#include "model/TransactionHistoryProxyModel.h"
#include "libwalletqt/Wallet.h"
#include "libwalletqt/WalletManager.h"
#include "utils/config.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

HistoryWidget::HistoryWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::HistoryWidget)
        , m_wallet(wallet)
        , m_contextMenu(new QMenu(this))
        , m_copyMenu(new QMenu("Copy", this))
        , m_model(wallet->historyModel())
{
    ui->setupUi(this);
    m_contextMenu->addMenu(m_copyMenu);
    m_contextMenu->addAction(icons()->icon("info2.svg"), "Show details", this, &HistoryWidget::showTxDetails);
    m_contextMenu->addAction("View on block explorer", this, &HistoryWidget::onViewOnBlockExplorer);

    // copy menu
    m_copyMenu->addAction("Transaction ID", this, [this]{copy(copyField::TxID);});
    m_copyMenu->addAction("Date", this, [this]{copy(copyField::Date);});
    m_copyMenu->addAction("Description", this, [this]{copy(copyField::Description);});
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
        conf()->set(Config::showHistorySyncNotice, false);
        ui->syncNotice->hide();
    });

    connect(m_wallet, &Wallet::walletRefreshed, this, &HistoryWidget::onWalletRefreshed);

    ui->syncNotice->setVisible(conf()->get(Config::showHistorySyncNotice).toBool());
    ui->history->setHistoryModel(m_model);

    // Load view state
    QByteArray historyViewState = QByteArray::fromBase64(conf()->get(Config::GUI_HistoryViewState).toByteArray());
    if (!historyViewState.isEmpty()) {
        ui->history->setViewState(historyViewState);
    }

    ui->btn_options->setMenu(ui->history->getMenu());
    ui->history->setColumnHidden(TransactionHistoryModel::FiatAmount, true);
}

void HistoryWidget::setSearchbarVisible(bool visible) {
    ui->frame_search->setVisible(visible);
}

void HistoryWidget::focusSearchbar() {
    ui->search->setFocusPolicy(Qt::StrongFocus);
    ui->search->setFocus();
}

void HistoryWidget::showContextMenu(const QPoint &point) {
    QModelIndex index = ui->history->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QMenu menu(this);

    auto txIdx = ui->history->getCurrentIndex();
    if (!txIdx.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(txIdx);

    bool unconfirmed = tx.failed || tx.pending;
    if (unconfirmed && tx.direction != TransactionRow::Direction_In) {
        menu.addAction("Resend transaction", this, &HistoryWidget::onResendTransaction);
    }

    if (tx.failed) {
        menu.addAction("Remove from history", this, &HistoryWidget::onRemoveFromHistory);
    }

    menu.addMenu(m_copyMenu);
    menu.addAction("Show details", this, &HistoryWidget::showTxDetails);
    menu.addAction("View on block explorer", this, &HistoryWidget::onViewOnBlockExplorer);
    menu.addAction("Create Tx Proof", this, &HistoryWidget::createTxProof);

    menu.exec(ui->history->viewport()->mapToGlobal(point));
}

void HistoryWidget::onResendTransaction() {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);
    emit resendTransaction(tx.hash);
}

void HistoryWidget::onRemoveFromHistory() {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);

    auto result = QMessageBox::question(this, "Remove transaction from history", "Are you sure you want to remove this transaction from the history?");
    if (result == QMessageBox::Yes) {
        m_wallet->removeFailedTx(tx.hash);
    }
}

void HistoryWidget::resetModel()
{
    // Save view state
    conf()->set(Config::GUI_HistoryViewState, ui->history->viewState().toBase64());
    conf()->sync();

    ui->history->setModel(nullptr);
}

void HistoryWidget::showTxDetails() {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);

    auto *dialog = new TxInfoDialog(m_wallet, tx, this);
    connect(dialog, &TxInfoDialog::resendTranscation, [this](const QString &txid){
       emit resendTransaction(txid);
    });
    dialog->show();
    dialog->setAttribute(Qt::WA_DeleteOnClose);
}

void HistoryWidget::onViewOnBlockExplorer() {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);

    emit viewOnBlockExplorer(tx.hash);
}

void HistoryWidget::setSearchText(const QString &text) {
    ui->search->setText(text);
}

void HistoryWidget::setSearchFilter(const QString &filter) {
    m_model->setSearchFilter(filter);
    ui->history->setSearchMode(!filter.isEmpty());
}

void HistoryWidget::createTxProof() {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);

    TxProofDialog dialog{this, m_wallet, tx};
    dialog.getTxKey();
    dialog.exec();
}

void HistoryWidget::copy(copyField field) {
    auto index = ui->history->getCurrentIndex();
    if (!index.isValid()) {
        return;
    }
    const TransactionRow& tx = ui->history->sourceModel()->entryFromIndex(index);

    QString data = [field, tx]{
        switch(field) {
            case copyField::TxID:
                return tx.hash;
            case copyField::Description:
                return tx.description;
            case copyField::Date:
                return tx.timestamp.toString(QString("%1 %2").arg(conf()->get(Config::dateFormat).toString(),
                                                                     conf()->get(Config::timeFormat).toString()));
            case copyField::Amount:
                return WalletManager::displayAmount(abs(tx.balanceDelta));
            default:
                return QString("");
        }
    }();

    Utils::copyToClipboard(data);
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

HistoryWidget::~HistoryWidget() = default;
