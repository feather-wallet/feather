// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_TXFIATHISTORY_H
#define FEATHER_TXFIATHISTORY_H

class TxFiatHistory : public QObject {
    Q_OBJECT

public:
    explicit TxFiatHistory(int genesis_timestamp, const QString &configDirectory, QObject *parent = nullptr);
    double get(const QString &date);
    double get(int timestamp);

public slots:
    void onUpdateDatabase();
    void onWSData(const QJsonObject &data);

signals:
    void requestYear(int year);
    void requestYearMonth(int year, int month);

private:
    void loadDatabase();
    void writeDatabase();
    int m_genesis_timestamp;
    QString m_databasePath;
    QString m_configDirectory;
    bool m_initialized = false;
    QMap<QString, double> m_database;
};

#endif //FEATHER_TXFIATHISTORY_H
