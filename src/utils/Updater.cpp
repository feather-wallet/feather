// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#include "Updater.h"

#include <common/util.h>
#undef config
#include <openpgp/hash.h>

#include "utils/config.h"
#include "config-feather.h"
#include "Utils.h"
#include "utils/AsyncTask.h"
#include "utils/networking.h"
#include "utils/NetworkManager.h"
#include "utils/SemanticVersion.h"

Updater::Updater(QObject *parent) :
    QObject(parent)
{
    std::string featherWallet = Utils::fileOpen(":/assets/gpg_keys/featherwallet.asc").toStdString();
    m_maintainers.emplace_back(featherWallet);
}

void Updater::checkForUpdates() {
    UtilsNetworking network{this};
    QNetworkReply *reply = network.getJson(QString("%1/updates.json").arg(this->getWebsiteUrl()));
    if (!reply) {
        emit updateCheckFailed("Can't check for websites: offline mode enabled");
        return;
    }

    connect(reply, &QNetworkReply::finished, this, std::bind(&Updater::onUpdateCheckResponse, this, reply));
}

void Updater::onUpdateCheckResponse(QNetworkReply *reply) {
    const QString err = reply->errorString();

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonObject updates;
    if (!data.isEmpty() && Utils::validateJSON(data)) {
        auto doc = QJsonDocument::fromJson(data);
        updates = doc.object();
    }
    else {
        qWarning() << err;
        emit updateCheckFailed(err);
        return;
    }

    this->wsUpdatesReceived(updates);
}

void Updater::wsUpdatesReceived(const QJsonObject &updates) {
    QString featherVersionStr{FEATHER_VERSION};

    auto featherVersion = SemanticVersion::fromString(featherVersionStr);

    QString platformTag = getPlatformTag();
    if (platformTag.isEmpty()) {
        QString err{"Unsupported platform, unable to fetch update"};
        emit updateCheckFailed(err);
        qWarning() << err;
        return;
    }

    QJsonObject platformData = updates["platform"].toObject()[platformTag].toObject();
    if (platformData.isEmpty()) {
        QString err{"Unable to find current platform in updates data"};
        emit updateCheckFailed(err);
        qWarning() << err;
        return;
    }

    QString newVersion = platformData["version"].toString();
    if (SemanticVersion::fromString(newVersion) <= featherVersion) {
        emit noUpdateAvailable();
        return;
    }

    // Hooray! New update available

    QString hashesUrl = QString("%1/files/releases/hashes-%2-plain.txt").arg(this->getWebsiteUrl(), newVersion);
    qDebug() << hashesUrl;

    UtilsNetworking network{this};
    QNetworkReply *reply = network.get(hashesUrl);

    connect(reply, &QNetworkReply::finished, this, std::bind(&Updater::onSignedHashesReceived, this, reply, platformTag, newVersion));
}

void Updater::onSignedHashesReceived(QNetworkReply *reply, const QString &platformTag, const QString &version) {
    if (reply->error() != QNetworkReply::NoError) {
        QString err{QString("Unable to fetch signed hashed: %1").arg(reply->errorString())};
        emit updateCheckFailed(err);
        qWarning() << err;
        return;
    }

    QByteArray armoredSignedHashes = reply->readAll();
    reply->deleteLater();

    const QString binaryFilename = QString("feather-%1-%2.zip").arg(version, platformTag);
    QByteArray signedHash{};
    QString signer;
    try {
         signedHash = this->verifyParseSignedHashes(armoredSignedHashes, binaryFilename, signer);
    }
    catch (const std::exception &e) {
        QString err{QString("Failed to fetch and verify signed hash: %1").arg(e.what())};
        emit updateCheckFailed(err);
        qWarning() << err;
        return;
    }

    QString hash = signedHash.toHex();
    qInfo() << "Update found: " << binaryFilename << hash << "signed by:" << signer;

    this->state = Updater::State::UPDATE_AVAILABLE;
    this->version = version;
    this->binaryFilename = binaryFilename;
    this->downloadUrl = QString("%1/files/releases/%2/%3").arg(this->getWebsiteUrl(), platformTag, binaryFilename);
    this->hash = hash;
    this->signer = signer;
    this->platformTag = platformTag;

    emit updateAvailable();
}

QString Updater::getPlatformTag() {
#ifdef Q_OS_MACOS
    return "mac";
#endif
#ifdef Q_OS_WIN
    #ifdef PLATFORM_INSTALLER
    return "win-installer";
#endif
    return "win";
#endif
#ifdef Q_OS_LINUX
    QString tag = "";

    QString arch = QSysInfo::buildCpuArchitecture();
    if (arch == "arm64") {
        tag += "linux-arm64";
    } else if (arch == "arm") {
        tag += "linux-arm";
    } else {
        tag += "linux";
    }

    if (!qEnvironmentVariableIsEmpty("APPIMAGE")) {
        tag += "-appimage";
    }

    return tag;
#endif
    return "";
}

QString Updater::getWebsiteUrl() {
        if (config()->get(Config::proxy).toInt() == Config::Proxy::Tor && config()->get(Config::torOnlyAllowOnion).toBool()) {
            return "http://featherdvtpi7ckdbkb2yxjfwx3oyvr3xjz3oo4rszylfzjdg6pbm3id.onion";
        }
        else if (config()->get(Config::proxy).toInt() == Config::Proxy::i2p) {
            return "http://rwzulgcql2y3n6os2jhmhg6un2m33rylazfnzhf56likav47aylq.b32.i2p";
        }
        else {
            return "https://featherwallet.org";
        }
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
