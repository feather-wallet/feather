//
// Created by dev on 7/29/24.
//

#ifndef FEATHER_ATOMICRECOVERDIALOG_H
#define FEATHER_ATOMICRECOVERDIALOG_H

#include <QDialog>
#include "components.h"
#include "History.h"

QT_BEGIN_NAMESPACE
namespace Ui { class AtomicRecoverDialog; }
QT_END_NAMESPACE

class AtomicRecoverDialog : public WindowModalDialog {
Q_OBJECT

public:
    explicit AtomicRecoverDialog(QWidget *parent = nullptr);
    bool historyEmpty();
    void appendHistory(HistoryEntry entry);
    ~AtomicRecoverDialog() override;

private:
    Ui::AtomicRecoverDialog *ui;
};


#endif //FEATHER_ATOMICRECOVERDIALOG_H
