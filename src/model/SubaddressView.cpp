// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "SubaddressView.h"

#include "utils/Utils.h"
#include "SubaddressModel.h"

SubaddressView::SubaddressView(QWidget *parent) : QTreeView(parent) {

}

void SubaddressView::keyPressEvent(QKeyEvent *event){
    QModelIndexList selectedRows = selectionModel()->selectedRows();

    if(!selectedIndexes().isEmpty()){
        if(event->matches(QKeySequence::Copy)){
            QModelIndex index = this->currentIndex();
            if (index.column() == SubaddressModel::ModelColumn::Address) {
                emit copyAddress();
            } else {
                Utils::copyColumn(&index, index.column());
            }
        }
        else
            QTreeView::keyPressEvent(event);
    }
}
