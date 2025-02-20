// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FEATHER_SEEDRECOVERYDIALOG_H
#define FEATHER_SEEDRECOVERYDIALOG_H

#include <QDialog>

#include "components.h"
#include "utils/scheduler.h"

namespace Ui {
    class SeedRecoveryDialog;
}

class SeedRecoveryDialog : public WindowModalDialog
{
Q_OBJECT

public:
    explicit SeedRecoveryDialog(QWidget *parent = nullptr);
    ~SeedRecoveryDialog() override;

signals:
    void progressUpdated(int value);
    void searchFinished(bool cancelled);
    void matchFound(QString match);
    void addressMatchFound(QString match);

private slots:
    void onFinished(bool cancelled);
    void onMatchFound(const QString &match);
    void onAddressMatchFound(const QString &match);
    void onProgressUpdated(int value);

private:
    void checkSeed();
    QStringList wordsWithRegex(const QRegularExpression &regex);
    bool isAlpha(const QString &word);
    bool findNext(const QList<QStringList> &words, QList<int> &index);
    QString mnemonic(const QList<QStringList> &words, const QList<int> &index);

    std::atomic<bool> m_cancelled = false;

    QStringList m_wordList;
    QFutureWatcher<void> m_watcher;
    FutureScheduler m_scheduler;
    QScopedPointer<Ui::SeedRecoveryDialog> ui;
};

#endif //FEATHER_SEEDRECOVERYDIALOG_H
