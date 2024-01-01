// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef MONERO_GUI_WALLETLISTENERIMPL_H
#define MONERO_GUI_WALLETLISTENERIMPL_H

#include "wallet/api/wallet2_api.h"
#include "PassphraseHelper.h"

class Wallet;

class WalletListenerImpl : public Monero::WalletListener, public PassphraseReceiver
{
public:
    WalletListenerImpl(Wallet * w);

    virtual void moneySpent(const std::string &txId, uint64_t amount) override;

    virtual void moneyReceived(const std::string &txId, uint64_t amount) override;

    virtual void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override;

    virtual void newBlock(uint64_t height) override;

    virtual void updated() override;

    // called when wallet refreshed by background thread or explicitly
    virtual void refreshed(bool success) override;

    virtual void onDeviceButtonRequest(uint64_t code) override;

    virtual void onDeviceButtonPressed() override;

    virtual void onDeviceError(const std::string &message) override;

    virtual void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) override;

    virtual Monero::optional<std::string> onDevicePassphraseRequest(bool & on_device) override;

private:
    Wallet * m_wallet;
    PassphraseHelper m_phelper;
};

#endif //MONERO_GUI_WALLETLISTENERIMPL_H
