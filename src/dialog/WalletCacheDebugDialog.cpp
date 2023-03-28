// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "WalletCacheDebugDialog.h"
#include "ui_WalletCacheDebugDialog.h"

#include <QRadioButton>

#include "utils/Utils.h"

WalletCacheDebugDialog::WalletCacheDebugDialog(Wallet *wallet, QWidget *parent)
        : WindowModalDialog(parent)
        , ui(new Ui::WalletCacheDebugDialog)
        , m_wallet(wallet)
{
    ui->setupUi(this);

    ui->output->setFont(Utils::getMonospaceFont());

    connect(ui->m_blockchain, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printBlockchain());
    });

    connect(ui->m_transfers, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printTransfers());
    });

    connect(ui->m_unconfirmed_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printUnconfirmedPayments());
    });

    connect(ui->m_confirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printConfirmedTransferDetails());
    });

    connect(ui->m_unconfirmed_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printUnconfirmedTransferDetails());
    });

    connect(ui->m_payments, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printPayments());
    });

    connect(ui->m_pub_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printPubKeys());
    });

    connect(ui->m_tx_notes, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printTxNotes());
    });

    connect(ui->m_subaddresses, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printSubaddresses());
    });

    connect(ui->m_subaddress_labels, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printSubaddressLabels());
    });

    connect(ui->m_additional_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printAdditionalTxKeys());
    });

    connect(ui->m_attributes, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printAttributes());
    });

    connect(ui->m_key_images, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printKeyImages());
    });

    connect(ui->m_account_tags, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printAccountTags());
    });

    connect(ui->m_tx_keys, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printTxKeys());
    });

    connect(ui->m_address_book, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printAddressBook());
    });

    connect(ui->m_scanned_pool_txs, &QRadioButton::pressed, [this]{
        this->setOutput(m_wallet->printScannedPoolTxs());
    });

    this->adjustSize();
}

void WalletCacheDebugDialog::setOutput(const QString &output) {
    ui->output->setPlainText(output);
}

WalletCacheDebugDialog::~WalletCacheDebugDialog() = default;
