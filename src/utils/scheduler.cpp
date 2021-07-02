// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "scheduler.h"

FutureScheduler::FutureScheduler(QObject *parent)
    : QObject(parent), Alive(0), Stopping(false)
{
    static std::once_flag once;
    std::call_once(once, []() {
        QThreadPool::globalInstance()->setMaxThreadCount(4);
    });
}

FutureScheduler::~FutureScheduler()
{
    shutdownWaitForFinished();
}

void FutureScheduler::shutdownWaitForFinished() noexcept
{
    QMutexLocker locker(&Mutex);

    Stopping = true;
    while (Alive > 0)
    {
        Condition.wait(&Mutex);
    }
}

QPair<bool, QFuture<void>> FutureScheduler::run(std::function<void()> function) noexcept
{
    return execute<void>([this, function](QFutureWatcher<void> *) {
        return QtConcurrent::run([this, function] {
            try
            {
                function();
            }
            catch (const std::exception &exception)
            {
                qWarning() << "Exception thrown from async function: " << exception.what();
            }
            done();
        });
    });
}

QPair<bool, QFuture<QVariantMap>> FutureScheduler::run(const std::function<QVariantMap()> &function, const std::function<void (QVariantMap)> &callback) noexcept
{
    return execute<QVariantMap>([this, function, callback](QFutureWatcher<QVariantMap> *watcher) {
        connect(watcher, &QFutureWatcher<QVariantMap>::finished, [watcher, callback] {
            callback(watcher->future().result());
        });
        return QtConcurrent::run([this, function] {
            QVariantMap result;
            try
            {
                result = function();
            }
            catch (const std::exception &exception)
            {
                qWarning() << "Exception thrown from async function: " << exception.what();
            }
            done();
            return result;
        });
    });
}

bool FutureScheduler::stopping() const noexcept
{
    return Stopping;
}

bool FutureScheduler::add() noexcept
{
    QMutexLocker locker(&Mutex);

    if (Stopping)
    {
        return false;
    }

    ++Alive;
    return true;
}

void FutureScheduler::done() noexcept
{
    {
        QMutexLocker locker(&Mutex);
        --Alive;
    }

    Condition.wakeAll();
}
