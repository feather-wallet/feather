// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef FUTURE_SCHEDULER_H
#define FUTURE_SCHEDULER_H

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QMutex>
#include <QPair>
#include <QWaitCondition>

class FutureScheduler : public QObject
{
    Q_OBJECT

public:
    FutureScheduler(QObject *parent);
    ~FutureScheduler();

    void shutdownWaitForFinished() noexcept;

    QPair<bool, QFuture<void>> run(std::function<void()> function) noexcept;
    QPair<bool, QFuture<QVariantMap>> run(const std::function<QVariantMap()>& function, const std::function<void (QVariantMap)>& callback) noexcept;

   // QPair<bool, QFuture<QJSValueList>> run(std::function<QJSValueList()> function, const QJSValue &callback);
   bool stopping() const noexcept;

private:
    bool add() noexcept;
    void done() noexcept;

    template<typename T>
    QPair<bool, QFuture<T>> execute(std::function<QFuture<T>(QFutureWatcher<T> *)> makeFuture) noexcept
    {
        if (add())
        {
            try
            {
                auto *watcher = new QFutureWatcher<T>();
                connect(watcher, &QFutureWatcher<T>::finished, [watcher] {
                    watcher->deleteLater();
                });
                watcher->setFuture(makeFuture(watcher));
                return qMakePair(true, watcher->future());
            }
            catch (const std::exception &exception)
            {
                qCritical() << "Failed to schedule async function: " << exception.what();
                done();
            }
        }

        return qMakePair(false, QFuture<T>());
    }

    QFutureWatcher<void> schedule(std::function<void()> function);
//    QFutureWatcher<QVariantMap> schedule(std::function<QVariantMap() noexcept> function, std::function<void> &callback);

private:
    size_t Alive;
    QWaitCondition Condition;
    QMutex Mutex;
    std::atomic<bool> Stopping;
};

#endif // FUTURE_SCHEDULER_H
