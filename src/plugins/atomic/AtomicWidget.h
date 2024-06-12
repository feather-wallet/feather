// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICWIDGET_H
#define FEATHER_ATOMICWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QTimer>
#include <QProcess>
#include <QMutex>

#include "OfferModel.h"
#include "AtomicSwap.h"
#include "config.h"

namespace Ui {
    class AtomicWidget;
}

class AtomicWidget : public QWidget
{
Q_OBJECT

public:
    explicit AtomicWidget(QWidget *parent = nullptr);
    ~AtomicWidget() override;
    void list(QString rendezvous);

public slots:
    void skinChanged();

private slots:
    void showAtomicConfigureDialog();
    void runSwap(QString seller, QString btcChange, QString xmrReceive);

private:
    void updateStatus();

    QScopedPointer<Ui::AtomicWidget> ui;
    bool m_comboBoxInit = false;
    QTimer m_statusTimer;
    OfferModel *o_model;
    QList<QSharedPointer<OfferEntry>> *offerList;
    AtomicSwap *swapDialog;

    void showAtomicSwapDialog();

    QList<QSharedPointer<QProcess>> *procList;
    Config *m_instance;
};

#endif // FEATHER_ATOMICWIDGET_H
