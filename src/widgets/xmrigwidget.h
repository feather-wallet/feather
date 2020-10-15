// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef XMRIGWIDGET_H
#define XMRIGWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QItemSelection>

#include "utils/xmrig.h"
#include "utils/config.h"

namespace Ui {
    class XMRigWidget;
}

class XMRigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit XMRigWidget(QWidget *parent = nullptr);
    ~XMRigWidget();
    QStandardItemModel *model();

public slots:
    void onStartClicked();
    void onStopClicked();
    void onClearClicked();
    void onDownloads(const QJsonObject &data);
    void linkClicked();

private slots:
    void onBrowseClicked();
    void onProcessError(const QString &msg);
    void onProcessOutput(const QByteArray &msg);
    void onThreadsValueChanged(int date);
    void onPoolChanged(int pos);
    void onHashrate(const QString &hashrate);

signals:
    void miningStarted();
    void miningEnded();

private:
    void showContextMenu(const QPoint &pos);

    Ui::XMRigWidget *ui;
    QStandardItemModel *m_model;
    QMenu *m_contextMenu;
    unsigned int m_threads;
    QStringList m_urls;
    QStringList m_pools{"pool.xmr.pt:5555", "pool.supportxmr.com:3333", "mine.xmrpool.net:3333", "xmrpool.eu:5555", "xmr-eu1.nanopool.org:14444", "pool.minexmr.com:4444", "monerohash.com:2222"};
    XMRig *m_rig;
};

#endif // REDDITWIDGET_H
