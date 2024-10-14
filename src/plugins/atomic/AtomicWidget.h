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
#include "AtomicFundDialog.h"
#include "AtomicRecoverDialog.h"

namespace Ui {
    class AtomicWidget;
}

class AtomicWidget : public QWidget
{
Q_OBJECT

public:
    explicit AtomicWidget(QWidget *parent = nullptr);
    ~AtomicWidget() override;
    void list(const QString& rendezvous);

public slots:
    void skinChanged();
    void clean();

private slots:
    void showAtomicConfigureDialog();
    void runSwap(const QString& seller, const QString& btcChange, const QString& xmrReceive);

private:
    QScopedPointer<Ui::AtomicWidget> ui;
    bool m_comboBoxInit = false;
    QTimer m_statusTimer;
    OfferModel *o_model = nullptr;
    QList<QSharedPointer<OfferEntry>> *offerList = nullptr;
    AtomicSwap *swapDialog = nullptr;
    AtomicFundDialog *fundDialog = nullptr;
    AtomicRecoverDialog *recoverDialog = nullptr;

    QList<QSharedPointer<QProcess>> *procList = nullptr;
};

#endif // FEATHER_ATOMICWIDGET_H
