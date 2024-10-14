// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_ATOMICSWAP_H
#define FEATHER_ATOMICSWAP_H

#include <QDialog>
#include <QTime>
#include <QProcess>
#include "components.h"
#include "AtomicFundDialog.h"


QT_BEGIN_NAMESPACE
namespace Ui { class AtomicSwap; }
QT_END_NAMESPACE

class AtomicSwap : public WindowModalDialog {
Q_OBJECT

public:
    explicit AtomicSwap(QWidget *parent = nullptr);
    void updateStatus(QString status);
    void logLine(QString line);
    ~AtomicSwap() override;
    void updateBTCConf(int confs);
    void updateXMRConf(int confs);
    void setTitle(QString title);
public slots:
    void runSwap(QStringList swap);
signals:
    void cleanProcs();
private:
    Ui::AtomicSwap *ui = nullptr;
    QString id;
    QString min;
    AtomicFundDialog* fundDialog;
    QList<QSharedPointer<QProcess>>* procList = nullptr;
    int btc_confs;
    void cancel();

};


#endif //FEATHER_ATOMICSWAP_H
