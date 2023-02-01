// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#pragma once

#include <QPair>
#include <QNetworkReply>

#include <openpgp/openpgp.h>

class Updater : public QObject
{
Q_OBJECT

public:
    enum State {
        NO_UPDATE = 0,
        UPDATE_AVAILABLE = 1
    };

public:
    explicit Updater(QObject *parent = nullptr);

    void checkForUpdates();

    QByteArray verifyParseSignedHashes(const QByteArray &armoredSignedHashes, const QString &binaryFilename, QString &signers) const;

    QByteArray getHash(const void *data, size_t size) const;
    QString verifySignature(const QByteArray &armoredSignedMessage, QString &signer) const;
    QByteArray parseShasumOutput(const QString &message, const QString &filename) const;

    State state = State::NO_UPDATE;
    QString version;
    QString binaryFilename;
    QString downloadUrl;
    QString hash;
    QString signer;
    QString platformTag;

signals:
    void updateCheckFailed(const QString &error);
    void noUpdateAvailable();
    void updateAvailable();

public slots:
    void onUpdateCheckResponse(QNetworkReply *reply);
    void wsUpdatesReceived(const QJsonObject &updates);

private:
    QString verifySignature(const epee::span<const uint8_t> data, const openpgp::signature_rsa &signature) const;
    void onSignedHashesReceived(QNetworkReply *reply, const QString &platformTag, const QString &version);
    QString getPlatformTag();

private:
    std::vector<openpgp::public_key_block> m_maintainers;
};
