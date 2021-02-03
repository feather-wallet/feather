// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WalletCacheDebugDialog.h"
#include "ui_WalletCacheDebugDialog.h"
#include "model/ModelUtils.h"

#include <QRadioButton>

WalletCacheDebugDialog::WalletCacheDebugDialog(AppContext *ctx, QWidget *parent)
        : QDialog(parent)
        , m_ctx(ctx)
        , ui(new Ui::WalletCacheDebugDialog)
{
    ui->setupUi(this);

    ui->output->setFont(ModelUtils::getMonospaceFont());

    connect(ui->m_blockchain, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printBlockchain());
    });

    connect(ui->m_transfers, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printTransfers());
    });

    connect(ui->m_unconfirmed_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printUnconfirmedPayments());
    });

    connect(ui->m_confirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printConfirmedTransferDetails());
    });

    connect(ui->m_unconfirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printUnconfirmedTransferDetails());
    });

    connect(ui->m_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printPayments());
    });

    connect(ui->m_pub_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printPubKeys());
    });

    connect(ui->m_tx_notes, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printTxNotes());
    });

    connect(ui->m_subaddresses, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printSubaddresses());
    });

    connect(ui->m_subaddress_labels, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printSubaddressLabels());
    });

    connect(ui->m_additional_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printAdditionalTxKeys());
    });

    connect(ui->m_attributes, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printAttributes());
    });

    connect(ui->m_key_images, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printKeyImages());
    });

    connect(ui->m_account_tags, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printAccountTags());
    });

    connect(ui->m_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printTxKeys());
    });

    connect(ui->m_address_book, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printAddressBook());
    });

    connect(ui->m_scanned_pool_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->currentWallet->printScannedPoolTxs());
    });

    this->adjustSize();
}

void WalletCacheDebugDialog::setOutput(const QString &output) {
    ui->output->setPlainText(output);
}

WalletCacheDebugDialog::~WalletCacheDebugDialog() {
    delete ui;
}

