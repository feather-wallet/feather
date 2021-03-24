// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#include <QNetworkAccessManager>
#include <iomanip>

#include "txfiathistory.h"
#include "utils/utils.h"

TxFiatHistory::TxFiatHistory(int genesis_timestamp, const QString &configDirectory, QObject *parent) :
        QObject(parent),
        m_genesis_timestamp(genesis_timestamp),
        m_configDirectory(configDirectory) {
    m_databasePath = QString("%1/fiatHistory.db").arg(configDirectory);
    this->loadDatabase();
}

double TxFiatHistory::get(int timestamp) {
    QDateTime ts;
    ts.setTime_t(timestamp);
    auto key = ts.toString("yyyyMMdd");
    return this->get(key);  // USD
}

double TxFiatHistory::get(const QString &date) {
    if(m_database.contains(date))
        return m_database[date];  // USD
    return 0.0;
}

void TxFiatHistory::loadDatabase() {
    if(!Utils::fileExists(m_databasePath))
        return;

    m_database.clear();
    QString contents = Utils::barrayToString(Utils::fileOpen(m_databasePath));
    for(auto &line: contents.split("\n")){
        line = line.trimmed();
        if(line.isEmpty()) continue;
        auto spl = line.split(":");
        m_database[spl.at(0)] = spl.at(1).toDouble();
    }
}

void TxFiatHistory::writeDatabase() {
    QString data;
    for(const auto &line: m_database.toStdMap())
        data += QString("%1:%2\n").arg(line.first).arg(QString::number(line.second));
    Utils::fileWrite(m_databasePath, data);
}

void TxFiatHistory::onUpdateDatabase() {
    // update local txFiatHistory database
    if(m_initialized) return;

    QDateTime genesis;
    genesis.setTime_t(m_genesis_timestamp);
    auto genesis_date = genesis.date();

    auto now = QDate::currentDate();
    auto nowKey = now.toString("yyyyMMdd");
    int year = genesis.toString("yyyy").toInt();
    auto yearCurrent = now.year();

    // if current year is genesis year we'll refresh regardless.
    if(yearCurrent == genesis_date.year()) {
        emit requestYear(year);
        m_initialized = true;
        return;
    }

    // keep local fiatTxHistory database up to date, loop for missing dates
    for(year; year != yearCurrent + 1; year += 1){
        for(int month = 1; month != 13; month++) {
            if(year == yearCurrent && month == now.month() && now.day() == 1) break;
            QDateTime _now;
            _now.setDate(QDate(year, month, 1));
            if(_now.toSecsSinceEpoch() < m_genesis_timestamp) continue;
            if(_now.toSecsSinceEpoch() > std::time(nullptr) - 86400) continue;
            QString key = "";

            // genesis year we'll only fetch once
            if(year == genesis_date.year()){
                key = QString("%1%2%3").arg(year).arg(12).arg("31");
                if(!m_database.contains(key))
                    emit requestYear(year);
                break;
            }

            auto _month = QString::number(month);
            if(_month.length() == 1)
                _month = QString("0%1").arg(_month);  // how2fill

            key = QString("%1%2%3").arg(year).arg(_month).arg("01");
            if(!m_database.contains(key)){
                if(year != yearCurrent) {
                    emit requestYear(year);
                    break;
                } else
                    emit requestYearMonth(year, month);
            } else if (year == yearCurrent && month == now.month() && !m_database.contains(nowKey))
                emit requestYearMonth(year, month);
        }
    }

    m_initialized = true;
}

void TxFiatHistory::onWSData(const QJsonObject &data) {
    foreach(const QString &key, data.keys()) {
        QJsonValue value = data.value(key);
        m_database[key] = value.toDouble();
    }

    this->writeDatabase();
}
