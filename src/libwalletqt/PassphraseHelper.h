// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#ifndef MONERO_GUI_PASSPHRASEHELPER_H
#define MONERO_GUI_PASSPHRASEHELPER_H

#include <QtGlobal>
#include <wallet/api/wallet2_api.h>
#include <QMutex>
#include <QPointer>
#include <QWaitCondition>
#include <QMutex>

/**
 * Implements component responsible for showing entry prompt to the user,
 * typically Wallet / Wallet manager.
 */
class PassprasePrompter {
public:
    virtual void onWalletPassphraseNeeded(bool onDevice) = 0;
};

/**
 * Implements receiver of the passphrase responsible for passing it back to the wallet,
 * typically wallet listener.
 */
class PassphraseReceiver {
public:
    virtual void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) = 0;
};

class PassphraseHelper {
public:
    PassphraseHelper(PassprasePrompter * prompter=nullptr): m_prompter(prompter) {};
    PassphraseHelper(const PassphraseHelper & h): PassphraseHelper(h.m_prompter) {};
    Monero::optional<std::string> onDevicePassphraseRequest(bool & on_device);
    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort);

private:
    PassprasePrompter * m_prompter;
    QWaitCondition m_cond_pass;
    QMutex m_mutex_pass;
    QString m_passphrase;
    bool m_passphrase_abort;
    bool m_passphrase_on_device;

};

#endif //MONERO_GUI_PASSPHRASEHELPER_H
