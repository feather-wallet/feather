// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_WALLETLISTENERIMPL_H
#define FEATHER_WALLETLISTENERIMPL_H

#include "wallet/api/wallet2_api.h"
#include "PassphraseHelper.h"

class Wallet;

class WalletListenerImpl : public Monero::WalletListener, public PassphraseReceiver
{
public:
    WalletListenerImpl(Wallet * w);

    void moneySpent(const std::string &txId, uint64_t amount) override;

    void moneyReceived(const std::string &txId, uint64_t amount, bool coinbase) override;

    void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override;

    void newBlock(uint64_t height) override;

    void updated() override;

    // called when wallet refreshed by background thread or explicitly
    void refreshed(bool success) override;

    void onDeviceButtonRequest(uint64_t code) override;

    void onDeviceButtonPressed() override;

    void onDeviceError(const std::string &message, unsigned int error_code) override;

    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) override;

    std::optional<std::string> onDevicePassphraseRequest(bool & on_device) override;

private:
    Wallet * m_wallet;
    PassphraseHelper m_phelper;
};

#endif //FEATHER_WALLETLISTENERIMPL_H
