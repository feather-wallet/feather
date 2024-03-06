// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_LEGACYSEEDRECOVERY_H
#define FEATHER_LEGACYSEEDRECOVERY_H


#include <QDialog>

#include "components.h"
#include "utils/scheduler.h"

#include "cryptonote_basic/account.h"

namespace Ui {
    class LegacySeedRecovery;
}

class LegacySeedRecovery : public WindowModalDialog
{
Q_OBJECT

public:
    explicit LegacySeedRecovery(QWidget *parent = nullptr);
    ~LegacySeedRecovery() override;

    enum Mode {
        WORD_24 = 0,
        WORD_25 = 1
    };

signals:
    void progressUpdated(int value);
    void searchFinished(bool cancelled);
    void matchFound(QString match);
    void addressMatchFound(QString match);
    void addResultText(QString text);

private slots:
    void onFinished(bool cancelled);
    void onMatchFound(const QString &match);
    void onAddressMatchFound(const QString &match);
    void onProgressUpdated(int value);
    void onAddResultText(const QString &text);

private:
    void checkSeed();
    QString mnemonic(const QList<QStringList> &words, const QList<int> &index);

    bool testSeed(const QString &seed, const crypto::public_key &spkey);

    std::atomic<bool> m_cancelled = false;

    int m_major = 50;
    int m_minor = 200;

    QHash<QString, QStringList> m_wordLists;
    QFutureWatcher<void> m_watcher;
    FutureScheduler m_scheduler;
    QScopedPointer<Ui::LegacySeedRecovery> ui;
};


#endif //FEATHER_LEGACYSEEDRECOVERY_H
