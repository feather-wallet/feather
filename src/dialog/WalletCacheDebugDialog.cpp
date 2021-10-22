// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include "WalletCacheDebugDialog.h"
#include "ui_WalletCacheDebugDialog.h"

#include <QRadioButton>

#include "model/ModelUtils.h"

WalletCacheDebugDialog::WalletCacheDebugDialog(QSharedPointer<AppContext> ctx, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::WalletCacheDebugDialog)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    ui->output->setFont(ModelUtils::getMonospaceFont());

    connect(ui->m_blockchain, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printBlockchain());
    });

    connect(ui->m_transfers, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printTransfers());
    });

    connect(ui->m_unconfirmed_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printUnconfirmedPayments());
    });

    connect(ui->m_confirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printConfirmedTransferDetails());
    });

    connect(ui->m_unconfirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printUnconfirmedTransferDetails());
    });

    connect(ui->m_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printPayments());
    });

    connect(ui->m_pub_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printPubKeys());
    });

    connect(ui->m_tx_notes, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printTxNotes());
    });

    connect(ui->m_subaddresses, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printSubaddresses());
    });

    connect(ui->m_subaddress_labels, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printSubaddressLabels());
    });

    connect(ui->m_additional_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printAdditionalTxKeys());
    });

    connect(ui->m_attributes, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printAttributes());
    });

    connect(ui->m_key_images, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printKeyImages());
    });

    connect(ui->m_account_tags, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printAccountTags());
    });

    connect(ui->m_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printTxKeys());
    });

    connect(ui->m_address_book, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printAddressBook());
    });

    connect(ui->m_scanned_pool_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_ctx->wallet->printScannedPoolTxs());
    });

    this->adjustSize();
}

void WalletCacheDebugDialog::setOutput(const QString &output) {
    ui->output->setPlainText(output);
}

WalletCacheDebugDialog::~WalletCacheDebugDialog() = default;
