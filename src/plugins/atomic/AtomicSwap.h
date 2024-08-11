//
// Created by dev on 6/11/24.
//

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
    Ui::AtomicSwap *ui;
    QString id;
    AtomicFundDialog* fundDialog;
    QList<QSharedPointer<QProcess>>* procList;
    int btc_confs;
    void cancel();

};


#endif //FEATHER_ATOMICSWAP_H
