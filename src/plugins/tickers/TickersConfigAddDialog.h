// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: The Monero Project

#ifndef TICKERSCONFIGADDDIALOG_H
#define TICKERSCONFIGADDDIALOG_H

#include <QDialog>
#include <QListWidget>

#include "components.h"

namespace Ui {
    class TickersConfigAddDialog;
}

class TickersConfigAddDialog : public WindowModalDialog
{
    Q_OBJECT

public:
    explicit TickersConfigAddDialog(QWidget *parent = nullptr);
    ~TickersConfigAddDialog() override;

    enum TickersType {
        TICKER = 0,
        RATIO
    };

    QString getTicker();

private:
    QScopedPointer<Ui::TickersConfigAddDialog> ui;
    TickersType m_type;
};

#endif //TICKERSCONFIGADDDIALOG_H
