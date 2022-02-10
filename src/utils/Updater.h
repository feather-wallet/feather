// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#pragma once

#include <QPair>
#include <QNetworkReply>

#include <openpgp/openpgp.h>

class Updater
{
public:
    explicit Updater();

    QByteArray verifyParseSignedHashes(const QByteArray &armoredSignedHashes, const QString &binaryFilename, QString &signers) const;

    QByteArray getHash(const void *data, size_t size) const;
    QString verifySignature(const QByteArray &armoredSignedMessage, QString &signer) const;
    QByteArray parseShasumOutput(const QString &message, const QString &filename) const;

private:
    QString verifySignature(const epee::span<const uint8_t> data, const openpgp::signature_rsa &signature) const;

private:
    std::vector<openpgp::public_key_block> m_maintainers;
};
