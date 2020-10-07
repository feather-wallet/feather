#ifndef FEATHER_TXFIATHISTORY_H
#define FEATHER_TXFIATHISTORY_H


class TxFiatHistory : public QObject {
    Q_OBJECT

public:
    explicit TxFiatHistory(unsigned int genesis_timestamp, const QString &configDirectory, QObject *parent = nullptr);
    double get(const QString &date);
    double get(unsigned int timestamp);

public slots:
    void onUpdateDatabase();
    void onWSData(const QJsonObject &data);

signals:
    void requestYear(unsigned int year);
    void requestYearMonth(unsigned int year, unsigned int month);

private:
    void loadDatabase();
    void writeDatabase();
    QString m_databasePath;
    QString m_configDirectory;
    bool m_initialized = false;
    QMap<QString, double> m_database;
    unsigned int m_genesis_timestamp;
};

#endif //FEATHER_TXFIATHISTORY_H
