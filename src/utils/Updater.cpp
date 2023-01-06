// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Updater.h"

#include <common/util.h>
#include <openpgp/hash.h>

#include "Utils.h"

Updater::Updater() {
    std::string featherWallet = Utils::fileOpen(":/assets/gpg_keys/featherwallet.asc").toStdString();
    m_maintainers.emplace_back(featherWallet);
}

QByteArray Updater::verifyParseSignedHashes(
        const QByteArray &armoredSignedHashes,
        const QString &binaryFilename,
        QString &signers) const
{
    const QString signedMessage = verifySignature(armoredSignedHashes, signers);

    return parseShasumOutput(signedMessage, binaryFilename);
}

QByteArray Updater::getHash(const void *data, size_t size) const
{
    QByteArray hash(sizeof(crypto::hash), 0);
    tools::sha256sum(static_cast<const uint8_t *>(data), size, *reinterpret_cast<crypto::hash *>(hash.data()));
    return hash;
}

QByteArray Updater::parseShasumOutput(const QString &message, const QString &filename) const
{
    for (const auto &line : message.split("\n"))
    {
        const auto trimmed = line.trimmed();
        if (trimmed.endsWith(filename))
        {
            const int pos = trimmed.indexOf(' ');
            if (pos != -1)
            {
                return QByteArray::fromHex(trimmed.left(pos).toUtf8());
            }
        }
        else if (trimmed.startsWith(filename))
        {
            const int pos = trimmed.lastIndexOf(' ');
            if (pos != -1)
            {
                return QByteArray::fromHex(trimmed.right(trimmed.size() - pos).toUtf8());
            }
        }
    }

    throw std::runtime_error("hash not found");
}

QString Updater::verifySignature(const QByteArray &armoredSignedMessage, QString &signer) const
{
    const std::string messageString = armoredSignedMessage.toStdString();

    const openpgp::message_armored signedMessage(messageString);
    signer = verifySignature(signedMessage, openpgp::signature_rsa::from_armored(messageString));

    const epee::span<const uint8_t> message = signedMessage;
    return QString(QByteArray(reinterpret_cast<const char *>(&message[0]), message.size()));
}

QString Updater::verifySignature(const epee::span<const uint8_t> data, const openpgp::signature_rsa &signature) const
{
    for (const auto &maintainer : m_maintainers)
    {
        for (const auto &public_key : maintainer)
        {
            try {
                if (signature.verify(data, public_key))
                {
                    return QString::fromStdString(maintainer.user_id());
                }
            }
            catch (const std::exception &e) {
                qWarning() << e.what();
            }
        }
    }

    throw std::runtime_error("not signed by a maintainer");
}
