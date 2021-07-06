// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "WalletKeysFilesModel.h"

#include "utils/Utils.h"
#include <QDir>
#include <QDateTime>
#include "config.h"

using namespace std::chrono;

WalletKeysFile::WalletKeysFile(const QFileInfo &info, int networkType, QString address)
    : m_fileName(info.fileName())
    , m_modified(getModified(info))
    , m_path(QDir::toNativeSeparators(info.absoluteFilePath()))
    , m_networkType(networkType)
    , m_address(std::move(address))
{
}

qint64 WalletKeysFile::getModified(const QFileInfo &info) {
    qint64 m = info.lastModified().toSecsSinceEpoch();

    QFileInfo cacheFile = QFileInfo(info.absoluteFilePath().replace(QRegExp(".keys$"), ""));
    qint64 cacheLastModified = cacheFile.lastModified().toSecsSinceEpoch();
    if (cacheFile.exists() && cacheLastModified > m) {
        m = cacheLastModified;
    }
    return m;
}

// Model

WalletKeysFilesModel::WalletKeysFilesModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    this->updateDirectories();
}

void WalletKeysFilesModel::clear() {
    beginResetModel();
    m_walletKeyFiles.clear();
    endResetModel();
}

void WalletKeysFilesModel::refresh() {
    this->clear();
    this->findWallets();
    endResetModel();
}

void WalletKeysFilesModel::updateDirectories() {
    m_walletDirectories.clear();

    QDir defaultWalletDir = QDir(Utils::defaultWalletDir());
    QString walletDir = defaultWalletDir.path();
    defaultWalletDir.cdUp();
    QString walletDirRoot = defaultWalletDir.path();

    m_walletDirectories << walletDir;
    m_walletDirectories << walletDirRoot;
    m_walletDirectories << QDir::homePath();

    QString walletDirectory = config()->get(Config::walletDirectory).toString();
    if (!walletDirectory.isEmpty())
        m_walletDirectories << walletDirectory;

    m_walletDirectories.removeDuplicates();
}

void WalletKeysFilesModel::findWallets() {
    qDebug() << "wallet .keys search initiated";
    auto now = high_resolution_clock::now();

    QRegExp rx("*.keys");
    rx.setPatternSyntax(QRegExp::Wildcard);
    QStringList walletPaths;

    for(auto i = 0; i != m_walletDirectories.length(); i++) {
        // Scan default wallet dir (~/Monero/)
        walletPaths << Utils::fileFind(rx, m_walletDirectories[i], 0, i == 0 ? 2 : 0, 200);
    }

    walletPaths.removeDuplicates();
    for(const auto &walletPath: walletPaths) {
        QFile walletPathFile(walletPath);
        if(walletPathFile.size() <= 0)
            continue;

        QFileInfo fileInfo(walletPath);
        const QString absPath = fileInfo.absoluteFilePath();
        const QString path = fileInfo.path();
        const QString baseName = fileInfo.baseName();
        const QString basePath = QString("%1/%2").arg(path).arg(baseName);
        QString addr = QString("");
        quint8 networkType = NetworkType::MAINNET;

        if (Utils::fileExists(basePath + ".address.txt")) {
            QFile file(basePath + ".address.txt");
            file.open(QFile::ReadOnly | QFile::Text);
            const QString _address = QTextCodec::codecForMib(106)->toUnicode(file.readAll());

            if (!_address.isEmpty()) {
                addr = _address;
                if (addr.startsWith("5") || addr.startsWith("7"))
                    networkType = NetworkType::STAGENET;
                else if (addr.startsWith("9") || addr.startsWith("B"))
                    networkType = NetworkType::TESTNET;
            }
            file.close();
        }

        this->addWalletKeysFile(WalletKeysFile(fileInfo, networkType, std::move(addr)));
    }

    auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - now).count();
    qDebug() << QString("wallet .keys search completed in %1 ms").arg(duration);
}

void WalletKeysFilesModel::addWalletKeysFile(const WalletKeysFile &walletKeysFile) {
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_walletKeyFiles.append(walletKeysFile);
    endInsertRows();
}

int WalletKeysFilesModel::rowCount(const QModelIndex & parent) const {
    return parent.isValid() ? 0 : m_walletKeyFiles.count();
}

int WalletKeysFilesModel::columnCount(const QModelIndex &) const {
    return Column::COUNT;
}

QVariant WalletKeysFilesModel::data(const QModelIndex &index, int role) const {
    if (index.row() < 0 || index.row() >= m_walletKeyFiles.count())
        return QVariant();

    const auto &walletKeyFile = m_walletKeyFiles[index.row()];

    if (role == Qt::DisplayRole) {
        switch(index.column()) {
            case Column::NetworkType: {
                auto c = static_cast<NetworkType::Type>(walletKeyFile.networkType());
                if (c == NetworkType::Type::STAGENET)
                    return QString("stage");
                else if (c == NetworkType::Type::TESTNET)
                    return QString("test");
                return QString("main");
            }
            case Column::FileName: {
                return walletKeyFile.fileName().replace(".keys", "");
            }
            case Column::Path: {
                auto fp = walletKeyFile.path();
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
                if (fp.startsWith(QDir::homePath())) {
                    fp = QString("~/%1").arg(fp.remove(0, QDir::homePath().length() + 1));
                }
#endif
                return fp;
            }
            case Column::Modified:
                return walletKeyFile.modified();
            default:
                break;
        }
    } else if(role == Qt::UserRole) {
        switch(index.column()) {
            case Column::NetworkType:
                return static_cast<int>(walletKeyFile.networkType());
            case Column::FileName:
                return walletKeyFile.fileName();
            case Column::Modified:
                return (int)walletKeyFile.modified();
            case Column::Path:
                return walletKeyFile.path();
            default:
                break;
        }
    }

    return QVariant();
}

QVariant WalletKeysFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch(section) {
            case NetworkType:
                return QString("Type");
            case FileName:
                return QString("Name");
            case Path:
                return QString("Path");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

// ProxyModel

WalletKeysFilesProxyModel::WalletKeysFilesProxyModel(QObject *parent, NetworkType::Type nettype)
    : QSortFilterProxyModel(parent)
    , m_nettype(nettype)
{
}

bool WalletKeysFilesProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex nettypeIndex = sourceModel()->index(sourceRow, WalletKeysFilesModel::NetworkType, sourceParent);
    NetworkType::Type nettype = static_cast<NetworkType::Type>(sourceModel()->data(nettypeIndex, Qt::UserRole).toInt());

    return (m_nettype == nettype);
}