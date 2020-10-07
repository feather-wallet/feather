// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2020, The Monero Project.

#ifndef KEYSFILES_H
#define KEYSFILES_H

#include "libwalletqt/WalletManager.h"
#include "utils/networktype.h"
#include "utils/utils.h"
#include <QtCore>

class WalletKeysFiles
{
public:
    WalletKeysFiles(const QFileInfo &info, quint8 networkType, QString address);

    QString fileName() const;
    qint64 modified() const;
    QString path() const;
    quint8 networkType() const;
    QString address() const;

private:
    QString m_fileName;
    qint64 m_modified;
    QString m_path;
    quint8 m_networkType;
    QString m_address;
};

class WalletKeysFilesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum ModelColumns {
        NetworkType = 0,
        FileName,
        Path,
        Modified
    };

    explicit WalletKeysFilesModel(AppContext *ctx, QObject *parent = nullptr);

    Q_INVOKABLE void refresh();
    Q_INVOKABLE void clear();

    void findWallets();
    void addWalletKeysFile(const WalletKeysFiles &walletKeysFile);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QStringList walletDirectories;

private:
    AppContext *m_ctx;
    QList<WalletKeysFiles> m_walletKeyFiles;
    QAbstractItemModel *m_walletKeysFilesItemModel;
    QSortFilterProxyModel m_walletKeysFilesModelProxy;
};

class WalletKeysFilesProxyModel : public QSortFilterProxyModel
{
Q_OBJECT
public:
    explicit WalletKeysFilesProxyModel(QObject *parent = nullptr);
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

#endif // KEYSFILES_H
