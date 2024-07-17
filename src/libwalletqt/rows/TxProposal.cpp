//
// Created by user on 3/20/24.
//

#include "TxProposal.h"

TxProposal::TxProposal(QObject *parent)
    : QObject(parent)
    , messageId(0)
    , txCount(0)
    , numSignatures(0)
{

}

QString TxProposal::errorString() {
    if (this->txCount == 0) {
        return {"Tx proposal contains no transactions"};
    }

    if (this->txCount > 1) {
        // the UI doesn't support split transactions
        return {"Tx proposal contains too many transactions"};
    }

    return "";
}