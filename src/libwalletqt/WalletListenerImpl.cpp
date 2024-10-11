// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "WalletListenerImpl.h"
#include "Wallet.h"
#include "WalletManager.h"

WalletListenerImpl::WalletListenerImpl(Wallet * w)
    : m_wallet(w)
    , m_phelper(w)
{

}

// Beware!
// Do not call non-signal m_wallet functions here
// Nothing here runs in the GUI thread

void WalletListenerImpl::moneySpent(const std::string &txId, uint64_t amount)
{
    // Outgoing tx included in a block
    QString qTxId = QString::fromStdString(txId);
    qDebug() << Q_FUNC_INFO << qTxId << " " << WalletManager::displayAmount(amount);

    emit m_wallet->moneySpent(qTxId, amount);
}

void WalletListenerImpl::moneyReceived(const std::string &txId, uint64_t amount, bool coinbase)
{
    // Incoming tx included in a block.
    QString qTxId = QString::fromStdString(txId);
    qDebug() << Q_FUNC_INFO << qTxId << " " << WalletManager::displayAmount(amount);

    emit m_wallet->moneyReceived(qTxId, amount, coinbase);
}

void WalletListenerImpl::unconfirmedMoneyReceived(const std::string &txId, uint64_t amount)
{
    // Incoming tx in pool
    QString qTxId = QString::fromStdString(txId);
    qDebug() << Q_FUNC_INFO << qTxId << " " << WalletManager::displayAmount(amount);

    emit m_wallet->unconfirmedMoneyReceived(qTxId, amount);
}

void WalletListenerImpl::newBlock(uint64_t height)
{
    // Called whenever a new block gets scanned by the wallet
    emit m_wallet->newBlock(height, m_wallet->daemonBlockChainTargetHeight());
}

void WalletListenerImpl::updated()
{
    emit m_wallet->updated();
}

// called when wallet refreshed by background thread or explicitly
void WalletListenerImpl::refreshed(bool success)
{
    QString message = m_wallet->errorString();
    emit m_wallet->refreshed(success, message);
}

void WalletListenerImpl::onDeviceButtonRequest(uint64_t code)
{
    qDebug() << __FUNCTION__;
    emit m_wallet->deviceButtonRequest(code);
}

void WalletListenerImpl::onDeviceButtonPressed()
{
    qDebug() << __FUNCTION__;
    emit m_wallet->deviceButtonPressed();
}

void WalletListenerImpl::onDeviceError(const std::string &message)
{
    qDebug() << __FUNCTION__;
    emit m_wallet->deviceError(QString::fromStdString(message));
}

void WalletListenerImpl::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort)
{
    qDebug() << __FUNCTION__;
    m_phelper.onPassphraseEntered(passphrase, enter_on_device, entry_abort);
}

std::optional<std::string> WalletListenerImpl::onDevicePassphraseRequest(bool & on_device)
{
    qDebug() << __FUNCTION__;
    return m_phelper.onDevicePassphraseRequest(on_device);
}
