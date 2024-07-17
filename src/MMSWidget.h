//
// Created by user on 1/3/24.
//

#ifndef FEATHER_MMSWIDGET_H
#define FEATHER_MMSWIDGET_H

#include <QMenu>
#include <QWidget>
#include <QSvgWidget>
#include <QTreeView>

#include "model/MultisigMessageModel.h"
#include "model/MultisigIncomingTxModel.h"
#include "libwalletqt/MultisigMessageStore.h"
#include "libwalletqt/Wallet.h"

namespace Ui {
    class MMSWidget;
}

class MMSWidget : public QWidget
{
Q_OBJECT

public:
    explicit MMSWidget(Wallet *wallet, QWidget *parent = nullptr);
    void setModel(MultisigMessageModel *model, MultisigIncomingTxModel *incomingTxModel, MultisigMessageStore *store);
    ~MMSWidget() override;

private:
    quint32 idAtPoint(QTreeView *tree, const QPoint &point);
    quint32 iddAtPoint(const QPoint &point);
    void showContextMenu(const QPoint &point);
    void showContexxtMenu(const QPoint &point);

    QScopedPointer<Ui::MMSWidget> ui;
    Wallet *m_wallet;

    MultisigMessageStore *m_store;
    MultisigMessageModel * m_model;
};

#endif //FEATHER_MMSWIDGET_H
