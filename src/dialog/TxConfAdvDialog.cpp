// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TxConfAdvDialog.h"
#include "ui_TxConfAdvDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>

#include "constants.h"
#include "dialog/QrCodeDialog.h"
#include "libwalletqt/Input.h"
#include "libwalletqt/Transfer.h"
#include "libwalletqt/WalletManager.h"
#include "qrcode/QrCode.h"
#include "utils/AppData.h"
#include "utils/ColorScheme.h"
#include "utils/config.h"
#include "utils/Utils.h"

TxConfAdvDialog::TxConfAdvDialog(Wallet *wallet, const QString &description, QWidget *parent, bool offline)
    : WindowModalDialog(parent)
    , ui(new Ui::TxConfAdvDialog)
    , m_wallet(wallet)
    , m_exportSignedMenu(new QMenu(this))
    , m_exportTxKeyMenu(new QMenu(this))
    , m_offline(offline)
    , m_description(description)
{
    ui->setupUi(this);

    m_exportSignedMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::signedCopy);
    m_exportSignedMenu->addAction("Save to file", this, &TxConfAdvDialog::signedSaveFile);
    ui->btn_exportSigned->setMenu(m_exportSignedMenu);

    m_exportTxKeyMenu->addAction("Copy to clipboard", this, &TxConfAdvDialog::txKeyCopy);
    ui->btn_exportTxKey->setMenu(m_exportTxKeyMenu);

    connect(ui->btn_sign, &QPushButton::clicked, this, &TxConfAdvDialog::signTransaction);
    connect(ui->btn_send, &QPushButton::clicked, this, &TxConfAdvDialog::broadcastTransaction);
    connect(ui->btn_close, &QPushButton::clicked, this, &TxConfAdvDialog::closeDialog);

    ui->amount->setFont(Utils::getMonospaceFont());
    ui->fee->setFont(Utils::getMonospaceFont());
    ui->total->setFont(Utils::getMonospaceFont());

    if (m_offline) {
        ui->txid->hide();
        ui->label_txid->hide();
    }

    ui->treeInputs->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeOutputs->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeInputs, &QTreeView::customContextMenuRequested, [this](const QPoint &point){
        this->setupContextMenu(point, ui->treeInputs);
    });
    connect(ui->treeOutputs, &QTreeView::customContextMenuRequested, [this](const QPoint &point){
        this->setupContextMenu(point, ui->treeOutputs);
    });

    this->adjustSize();
}

void TxConfAdvDialog::setTransaction(PendingTransaction *tx, bool isSigned) {
    ui->btn_sign->hide();

    if (!isSigned) {
        ui->btn_exportSigned->hide();
        ui->btn_send->hide();
    }

    m_tx = tx;
    m_tx->refresh();
    PendingTransactionInfo *ptx = m_tx->transaction(0); //Todo: support split transactions

    // TODO: implement hasTxKey()
    if (!m_wallet->isHwBacked() && m_tx->transaction(0)->txKey() == "0100000000000000000000000000000000000000000000000000000000000000") {
        ui->btn_exportTxKey->hide();
    }

    m_txid = tx->txid().first();
    ui->txid->setText(m_txid);

    this->setAmounts(tx->amount(), tx->fee());

    this->setupConstructionData(ptx);
}

void TxConfAdvDialog::setUnsignedTransaction(UnsignedTransaction *utx) {
    m_utx = utx;
    m_utx->refresh();

    ui->btn_exportSigned->hide();
    ui->btn_exportTxKey->hide();
    ui->btn_sign->show();
    ui->btn_send->hide();

    ui->txid->setText("n/a");

    this->setAmounts(utx->amount(0), utx->fee(0));

    ConstructionInfo *ci = m_utx->constructionInfo(0);
    this->setupConstructionData(ci);
}

void TxConfAdvDialog::setAmounts(quint64 amount, quint64 fee) {
    QString preferredCur = conf()->get(Config::preferredFiatCurrency).toString();

    auto convert = [preferredCur](double amount){
        return QString::number(appData()->prices.convert("XMR", preferredCur, amount), 'f', 2);
    };

    QString amount_str = WalletManager::displayAmount(amount);
    QString fee_str = WalletManager::displayAmount(fee);
    QString total = WalletManager::displayAmount(amount + fee);
    QVector<QString> amounts = {amount_str, fee_str, total};
    int maxLength = Utils::maxLength(amounts);
    std::for_each(amounts.begin(), amounts.end(), [maxLength](QString& amount){amount = amount.rightJustified(maxLength, ' ');});

    QString amount_fiat = convert(amount / constants::cdiv);
    QString fee_fiat = convert(fee / constants::cdiv);
    QString total_fiat = convert((amount + fee) / constants::cdiv);
    QVector<QString> amounts_fiat = {amount_fiat, fee_fiat, total_fiat};
    int maxLengthFiat = Utils::maxLength(amounts_fiat);
    std::for_each(amounts_fiat.begin(), amounts_fiat.end(), [maxLengthFiat](QString& amount){amount = amount.rightJustified(maxLengthFiat, ' ');});

    if (m_offline) {
        ui->amount->setText(amount_str);
        ui->fee->setText(fee_str);
        ui->total->setText(total);
    } else {
        ui->amount->setText(QString("%1 (%2 %3)").arg(amounts[0], amounts_fiat[0], preferredCur));
        ui->fee->setText(QString("%1 (%2 %3)").arg(amounts[1], amounts_fiat[1], preferredCur));
        ui->total->setText(QString("%1 (%2 %3)").arg(amounts[2], amounts_fiat[2], preferredCur));   
    }
}

void TxConfAdvDialog::setupConstructionData(ConstructionInfo *ci) {
    for (const auto &in: ci->inputs()) {
        auto *item = new QTreeWidgetItem(ui->treeInputs);
        item->setText(0, in->pubKey());
        item->setFont(0, Utils::getMonospaceFont());
        item->setText(1, WalletManager::displayAmount(in->amount()));
    }
    ui->treeInputs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeInputs->resizeColumnToContents(1);
    ui->treeInputs->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->label_inputs->setText(QString("Inputs (%1)").arg(QString::number(ci->inputs().size())));

    for (const auto &out: ci->outputs()) {
        auto *item = new QTreeWidgetItem(ui->treeOutputs);
        item->setText(0, out->address());
        item->setText(1, WalletManager::displayAmount(out->amount()));
        item->setFont(0, Utils::getMonospaceFont());
        auto index = m_wallet->subaddressIndex(out->address());
        QBrush brush;
        if (index.isChange()) {
            brush = QBrush(ColorScheme::YELLOW.asColor(true));
            item->setToolTip(0, "Wallet change/primary address");
            // item->setHidden(true);
        }
        else if (index.isValid()) {
            brush = QBrush(ColorScheme::GREEN.asColor(true));
            item->setToolTip(0, "Wallet receive address");
        }
        else if (out->amount() == 0) {
            brush = QBrush(ColorScheme::GRAY.asColor(true));
            item->setToolTip(0, "Dummy output (Min. 2 outs consensus rule)");
        }
        item->setBackground(0, brush);
    }
    ui->treeOutputs->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeOutputs->resizeColumnToContents(1);
    ui->treeOutputs->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    ui->label_outputs->setText(QString("Outputs (%1)").arg(QString::number(ci->outputs().size())));

    this->adjustSize();
}

void TxConfAdvDialog::signTransaction() {
    this->accept();
}

void TxConfAdvDialog::signedSaveFile() {
    QString defaultName = QString("%1_signed_monero_tx").arg(QString::number(QDateTime::currentSecsSinceEpoch()));
    QString fn = QFileDialog::getSaveFileName(this, "Save transaction to file", QDir::home().filePath(defaultName), "Transaction (*signed_monero_tx)");
    if (fn.isEmpty()) {
        return;
    }

    bool success = m_tx->saveToFile(fn);

    if (success) {
        Utils::showInfo(this, "Transaction saved successfully");
    } else {
        Utils::showError(this, "Failed to save transaction to file");
    }
}

void TxConfAdvDialog::signedCopy() {
    Utils::copyToClipboard(m_tx->signedTxToHex(0));
}

void TxConfAdvDialog::txKeyCopy() {
    if (m_wallet->isHwBacked()) {
        Utils::showError(this, "Unable to copy transaction private key", "Function not supported for hardware wallets");
        return;
    }

    Utils::copyToClipboard(m_tx->transaction(0)->txKey());
}

void TxConfAdvDialog::broadcastTransaction() {
    if (m_tx == nullptr) return;
    m_wallet->commitTransaction(m_tx, m_description);
    QDialog::accept();
}

void TxConfAdvDialog::closeDialog() {
    if (m_tx != nullptr)
        m_wallet->disposeTransaction(m_tx);
    if (m_utx != nullptr)
        m_wallet->disposeTransaction(m_utx);
    QDialog::reject();
}

void TxConfAdvDialog::setupContextMenu(const QPoint &point, QTreeWidget *tree) {
    if (!tree) {
        return;
    }

    QTreeWidgetItem *header = tree->headerItem();
    if (!header) {
        return;
    }

    auto* menu = new QMenu(this);
    for (int column = 0; column < tree->columnCount(); column++)
    {
        menu->addAction(QString("Copy %1").arg(header->text(column)), this, [this, column, point, tree]{
            this->copyFromTree(point, column, tree);
        });
    }

    menu->popup(tree->viewport()->mapToGlobal(point));
}

void TxConfAdvDialog::copyFromTree(const QPoint &point, int column, QTreeWidget *tree) {
    QModelIndex index = tree->indexAt(point);
    if (!index.isValid()) {
        return;
    }

    QModelIndex dataIndex = index.sibling(index.row(), column);
    Utils::copyToClipboard(dataIndex.data().toString());
}

TxConfAdvDialog::~TxConfAdvDialog() = default;