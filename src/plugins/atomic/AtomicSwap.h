//
// Created by dev on 6/11/24.
//

#ifndef FEATHER_ATOMICSWAP_H
#define FEATHER_ATOMICSWAP_H

#include <QDialog>
#include <QTime>


QT_BEGIN_NAMESPACE
namespace Ui { class AtomicSwap; }
QT_END_NAMESPACE

class AtomicSwap : public QDialog {
Q_OBJECT

public:
    explicit AtomicSwap(QWidget *parent = nullptr);
    void updateStatus(QString status);
    void logLine(QString line);
    ~AtomicSwap() override;

private:
    Ui::AtomicSwap *ui;


};


#endif //FEATHER_ATOMICSWAP_H
