// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "CoinsInfo.h"
#include "libwalletqt/WalletManager.h"

QString CoinsInfo::displayAmount() const
{
    return WalletManager::displayAmount(amount);
}

QString CoinsInfo::getAddressLabel() const {
    if (subaddrIndex == 0) {
        if (coinbase) {
            return "Coinbase";
        }
        if (change) {
            return "Change";
        }
        if (addressLabel == "Primary account") {
            return "Primary address";
        }
    }

    return addressLabel;
}


void CoinsInfo::setUnlocked(bool unlocked_) {
    unlocked = unlocked_;
}

CoinsInfo::CoinsInfo()
        : blockHeight(0)
        , internalOutputIndex(0)
        , globalOutputIndex(0)
        , spent(false)
        , frozen(false)
        , spentHeight(0)
        , amount(0)
        , rct(false)
        , keyImageKnown(false)
        , pkIndex(0)
        , subaddrIndex(0)
        , subaddrAccount(0)
        , unlockTime(0)
        , unlocked(false)
        , coinbase(false)
        , change(false)
{

}
