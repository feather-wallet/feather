// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_XMRIGWIDGET_H
#define FEATHER_XMRIGWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "appcontext.h"
#include "utils/xmrig.h"
#include "utils/config.h"

namespace Ui {
    class XMRigWidget;
}

class XMRigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMRigWidget(QSharedPointer<AppContext> ctx, QWidget *parent = nullptr);
    ~XMRigWidget() override;
    QStandardItemModel *model();

    bool isMining();

public slots:
    void onWalletClosed();
    void onStartClicked();
    void onStopClicked();
    void onClearClicked();
    void onDownloads(const QJsonObject &data);
    void linkClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onHashrate(const QString &hashrate);
    void onSoloChecked(int state);

private slots:
    void onBrowseClicked();
    void onThreadsValueChanged(int date);
    void onPoolChanged(int pos);

signals:
    void miningStarted();
    void miningEnded();

private:
    void showContextMenu(const QPoint &pos);

    QScopedPointer<Ui::XMRigWidget> ui;
    QSharedPointer<AppContext> m_ctx;
    XmRig * m_XMRig;
    QStandardItemModel *m_model;
    QMenu *m_contextMenu;

    bool m_isMining = false;
    int m_threads;
    QStringList m_urls;
    QStringList m_pools{"pool.xmr.pt:9000", "pool.supportxmr.com:9000", "mine.xmrpool.net:443", "xmrpool.eu:9999", "xmr-eu1.nanopool.org:14433", "pool.minexmr.com:6666", "us-west.minexmr.com:6666", "monerohash.com:9999", "cryptonote.social:5555", "cryptonote.social:5556"};
};

#endif // FEATHER_XMRWIDGET_H
