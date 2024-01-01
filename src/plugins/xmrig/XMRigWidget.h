// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_XMRIGWIDGET_H
#define FEATHER_XMRIGWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>
#include <QStandardItemModel>

#include "xmrig.h"
#include "utils/config.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class XMRigWidget;
}

class XMRigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMRigWidget(Wallet *wallet, QWidget *parent = nullptr);
    ~XMRigWidget() override;
    QStandardItemModel *model();

    bool isMining();
    void setDownloadsTabEnabled(bool enabled);

public slots:
    void onWalletClosed();
    void onStartClicked();
    void onStopClicked();
    void onClearClicked();
    void onUsePrimaryAddressClicked();
    void onDownloads(const QJsonObject &data);
    void linkClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onHashrate(const QString &hashrate);
    void onMiningModeChanged(int mode);
    void onNetworkTLSToggled(bool checked);
    void onNetworkTorToggled(bool checked);
    void onXMRigStateChanged(QProcess::ProcessState state);

private slots:
    void onBrowseClicked();
    void onThreadsValueChanged(int date);
    void onPoolChanged(const QString &pool);
    void onXMRigElevationChanged(bool elevated);

signals:
    void miningStarted();
    void miningEnded();

private:
    void showContextMenu(const QPoint &pos);
    void updatePools();
    void printConsoleInfo();
    void setMiningStopped();
    void setMiningStarted();
    bool checkXMRigPath();

    QScopedPointer<Ui::XMRigWidget> ui;
    Wallet *m_wallet;
    XmRig *m_XMRig;
    QStandardItemModel *m_model;
    QMenu *m_contextMenu;

    bool m_isMining = false;
    QStringList m_urls;
    QStringList m_defaultPools{"pool.xmr.pt:9000", "pool.supportxmr.com:9000", "mine.xmrpool.net:443", "xmrpool.eu:9999", "xmr-eu1.nanopool.org:14433","monerohash.com:9999", "cryptonote.social:5555", "cryptonote.social:5556"};
};

#endif // FEATHER_XMRWIDGET_H
