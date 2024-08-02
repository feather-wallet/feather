//
// Created by dev on 7/29/24.
//

#ifndef FEATHER_HISTORY_H
#define FEATHER_HISTORY_H

#include <QString>
#include <QDateTime>

struct HistoryEntry {
    QDateTime timestamp;
    QString id;
};

Q_DECLARE_METATYPE(HistoryEntry);
#endif //FEATHER_HISTORY_H
