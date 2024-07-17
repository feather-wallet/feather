// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "MMSWidget.h"
#include "ui_MMSWidget.h"

#include <QMessageBox>
#include <QHeaderView>

#include "dialog/OutputInfoDialog.h"
#include "dialog/OutputSweepDialog.h"
#include "utils/Icons.h"
#include "utils/Utils.h"

MMSWidget::MMSWidget(Wallet *wallet, QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::MMSWidget)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    connect(ui->btn_mmsNext, &QPushButton::clicked, [this]{
        m_store->next();
    });

    connect(ui->btn_mmsNextSync, &QPushButton::clicked, [this]{
       m_store->next(true);
    });

    connect(ui->btn_exportMultisig, &QPushButton::clicked, [this]{
       m_store->exportMultisig();
    });

    connect(ui->btn_deleteAll, &QPushButton::clicked, [this]{
       m_store->deleteAllMessages();
    });

    ui->mms->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mms, &QTreeView::customContextMenuRequested, this, &MMSWidget::showContextMenu);
    connect(ui->transactionProposals, &QTreeView::customContextMenuRequested, this, &MMSWidget::showContexxtMenu);

    connect(m_wallet->mmsStore(), &MultisigMessageStore::connectionError, [this]{
       Utils::showError(this, "Unable to connect to message service");
    });

//    ui->tabWidget->setTabVisible(0, false);
}

void MMSWidget::setModel(MultisigMessageModel * model, MultisigIncomingTxModel * incomingModel, MultisigMessageStore * store) {
    m_store = store;
    m_model = model;
    ui->mms->setModel(m_model);

    ui->mms->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->mms->setSortingEnabled(true);

    ui->mms->setColumnHidden(MultisigMessageModel::Modified, true);
    ui->mms->setColumnHidden(MultisigMessageModel::Sent, true);
    ui->mms->setColumnHidden(MultisigMessageModel::Hash, true);
    ui->mms->setColumnHidden(MultisigMessageModel::TransportId, true);
    ui->mms->setColumnHidden(MultisigMessageModel::WalletHeight, true);
    ui->mms->setColumnHidden(MultisigMessageModel::SignatureCount, true);

    ui->transactionProposals->setModel(incomingModel);
    ui->transactionProposals->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->transactionProposals->setSortingEnabled(true);
    ui->transactionProposals->setColumnHidden(MultisigIncomingTxModel::MessageID, true);
    ui->transactionProposals->setColumnHidden(MultisigIncomingTxModel::TxCount, true);
    ui->transactionProposals->setColumnHidden(MultisigIncomingTxModel::TxId, true);
    ui->transactionProposals->setColumnHidden(MultisigIncomingTxModel::Signatures, true);
}

void MMSWidget::showContextMenu(const QPoint &point) {
    auto* menu = new QMenu(this);

    menu->addAction("Sign transaction", [this, point]{
        quint32 id = this->idAtPoint(ui->mms, point);
        m_store->signTx(id);
    });

    menu->addAction("Delete message", [this, point]{
        quint32 id = this->idAtPoint(ui->mms, point);
        m_store->deleteMessage(id);
    });

    menu->addAction("Copy message", [this, point]{
        quint32 id = this->idAtPoint(ui->mms, point);
        std::string content = m_store->exportMessage(id);
        Utils::copyToClipboard(QString::fromStdString(content));
    });

    menu->addAction("Export message", [this, point]{
        quint32 id = this->idAtPoint(ui->mms, point);
        std::string content = m_store->exportMessage(id);

        QString defaultName = QString("mms_message_content_%1.mms").arg(QString::number(id));
        QString fn = Utils::getSaveFileName(this, "Save key images to file", defaultName, "Key Images (*_keyImages)");
        if (fn.isEmpty()) {
            return;
        }

        QFile file{fn};
        if (!file.open(QIODevice::WriteOnly)) {
            Utils::showError(this, "Failed to export MMS message", QString("Could not open file %1 for writing").arg(fn));
            return;
        }

        file.write(content.data(), content.size());
        file.close();
    });

    menu->popup(ui->mms->viewport()->mapToGlobal(point));
}

void MMSWidget::showContexxtMenu(const QPoint &point) {
    auto* menu = new QMenu(this);

    menu->addAction("Review transaction", [this, point]{
        quint32 id = iddAtPoint(point);
        m_store->signTx(id);
    });

    menu->addAction("Remove transaction", [this, point]{
        quint32 id = iddAtPoint(point);
        m_store->deleteMessage(id);
    });

    menu->popup(ui->transactionProposals->viewport()->mapToGlobal(point));
}

quint32 MMSWidget::iddAtPoint(const QPoint &point) {
    QModelIndex index = ui->transactionProposals->indexAt(point);
    if (!index.isValid()) {
        return 0;
    }

    QModelIndex dataIndex = index.sibling(index.row(), MultisigIncomingTxModel::MessageID);

    return dataIndex.data().toUInt();
}

quint32 MMSWidget::idAtPoint(QTreeView *tree, const QPoint &point) {
    QModelIndex index = tree->indexAt(point);
    if (!index.isValid()) {
        return 0;
    }

    QModelIndex dataIndex = index.sibling(index.row(), MultisigMessageModel::Id);
    return dataIndex.data().toUInt();
}

MMSWidget::~MMSWidget() = default;