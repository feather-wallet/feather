// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "PassphraseHelper.h"
#include <QDebug>

Monero::optional<std::string> PassphraseHelper::onDevicePassphraseRequest(bool & on_device)
{
    qDebug() << __FUNCTION__;
    QMutexLocker locker(&m_mutex_pass);
    m_passphrase_on_device = true;
    m_passphrase_abort = false;

    if (m_prompter != nullptr){
        m_prompter->onWalletPassphraseNeeded(on_device);
    }

    m_cond_pass.wait(&m_mutex_pass);

    if (m_passphrase_abort)
    {
        throw std::runtime_error("Passphrase entry abort");
    }

    on_device = m_passphrase_on_device;
    if (!on_device) {
        auto tmpPass = m_passphrase.toStdString();
        m_passphrase = QString();
        return Monero::optional<std::string>(tmpPass);
    } else {
        return Monero::optional<std::string>();
    }
}

void PassphraseHelper::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort)
{
    qDebug() << __FUNCTION__;
    QMutexLocker locker(&m_mutex_pass);
    m_passphrase = passphrase;
    m_passphrase_abort = entry_abort;
    m_passphrase_on_device = enter_on_device;

    m_cond_pass.wakeAll();
}
