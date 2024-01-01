// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WALLETKEYSFILESMODEL_H
#define FEATHER_WALLETKEYSFILESMODEL_H

#include <QObject>
#include <QFileInfo>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "utils/networktype.h"

class WalletKeysFile
{
public:
    WalletKeysFile(const QFileInfo &info, int networkType, QString address);

    QString fileName() const {return m_fileName;};
    qint64 modified() const {return m_modified;};
    QString path() const {return m_path;};
    int networkType() const {return m_networkType;};
    QString address() const {return m_address;};

private:
    static qint64 getModified(const QFileInfo &info);

    QString m_fileName;
    qint64 m_modified;
    QString m_path;
    int m_networkType;
    QString m_address;
};

class WalletKeysFilesModel : public QAbstractTableModel
{
Q_OBJECT

public:
    enum Column {
        NetworkType = 0,
        FileName,
        Path,
        Modified,
        COUNT
    };

    explicit WalletKeysFilesModel(QObject *parent = nullptr);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clear();

    void findWallets();
    void addWalletKeysFile(const WalletKeysFile &walletKeysFile);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

private:
    void updateDirectories();

    QStringList m_walletDirectories;

    QList<WalletKeysFile> m_walletKeyFiles;
};

class WalletKeysFilesProxyModel : public QSortFilterProxyModel
{
Q_OBJECT

public:
    explicit WalletKeysFilesProxyModel(QObject *parent, NetworkType::Type nettype);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    NetworkType::Type m_nettype;
};

#endif //FEATHER_WALLETKEYSFILESMODEL_H
