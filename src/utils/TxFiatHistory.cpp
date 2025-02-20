// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#include "TxFiatHistory.h"

#include <QJsonObject>

#include "utils/Utils.h"

TxFiatHistory::TxFiatHistory(int genesis_timestamp, const QString &configDirectory, QObject *parent)
    : QObject(parent)
    , m_genesis_timestamp(genesis_timestamp)
    , m_databasePath(QString("%1/fiatHistory.db").arg(configDirectory))
{
    this->loadDatabase();
}

double TxFiatHistory::get(int timestamp) {
    QDateTime ts;
    ts.setSecsSinceEpoch(timestamp);
    auto key = ts.toString("yyyyMMdd");
    return this->get(key);  // USD
}

double TxFiatHistory::get(const QString &date) {
    if (m_database.contains(date)) {
        return m_database[date];  // USD
    }
    return 0.0;
}

void TxFiatHistory::loadDatabase() {
    if (!Utils::fileExists(m_databasePath)) {
        return;
    }

    m_database.clear();

    QString contents = Utils::barrayToString(Utils::fileOpen(m_databasePath));
    for (auto &line: contents.split("\n")) {
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        QStringList spl = line.split(":");
        if (spl.length() == 2) {
            m_database[spl.at(0)] = spl.at(1).toDouble();
        }
    }
}

void TxFiatHistory::writeDatabase() {
    QString data;
    for (const auto &line: m_database.toStdMap()) {
        data += QString("%1:%2\n").arg(line.first).arg(QString::number(line.second));
    }
    Utils::fileWrite(m_databasePath, data);
}

void TxFiatHistory::onUpdateDatabase() {
    // update local txFiatHistory database
    if (m_initialized) {
        return;
    }

    QDateTime genesis;
    genesis.setSecsSinceEpoch(m_genesis_timestamp);
    QDate genesis_date = genesis.date();

    QDate now = QDate::currentDate();

    QSet<int> missingYears;
    for (QDate date = genesis_date; date <= now;) {

        if (!m_database.contains(this->dateToKey(date))) {
            qInfo() << "TxFiatHistory: Can't find value for date: " << this->dateToKey(date);
            missingYears << date.year();
            date.setDate(date.year()+1, 1, 1);
            continue;
        }

        date = date.addDays(1);
    }

    for (const int year : missingYears) {
        emit requestYear(year);
    }

    m_initialized = true;
}

QString TxFiatHistory::dateToKey(const QDate &date) {
    return date.toString("yyyyMMdd");
}

void TxFiatHistory::onWSData(const QJsonObject &data) {
    foreach(const QString &key, data.keys()) {
        QJsonValue value = data.value(key);
        m_database[key] = value.toDouble();
    }

    this->writeDatabase();
}
