// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_PASSPHRASEHELPER_H
#define FEATHER_PASSPHRASEHELPER_H

#include <QMutex>
#include <QPointer>
#include <QWaitCondition>

/**
 * Implements component responsible for showing entry prompt to the user,
 * typically Wallet / Wallet manager.
 */
class PassphrasePrompter {
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
    PassphraseHelper(PassphrasePrompter * prompter=nullptr): m_prompter(prompter) {};
    PassphraseHelper(const PassphraseHelper & h): PassphraseHelper(h.m_prompter) {};
    std::optional<std::string> onDevicePassphraseRequest(bool & on_device);
    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort);

private:
    PassphrasePrompter * m_prompter;
    QWaitCondition m_cond_pass;
    QMutex m_mutex_pass;
    QString m_passphrase;
    bool m_passphrase_abort;
    bool m_passphrase_on_device;

};

#endif //FEATHER_PASSPHRASEHELPER_H
