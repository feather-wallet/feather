// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_WIZARDRESTORE_H
#define FEATHER_WIZARDRESTORE_H

#include <QtCore>
#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "utils/textedit.h"
#include "appcontext.h"

namespace Ui {
    class RestorePage;
}

class RestorePage : public QWizardPage
{
    Q_OBJECT

public:
    explicit RestorePage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;
    void cleanupPage() const;

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::RestorePage *ui;

    unsigned int m_mode = 14;
    QStringList m_words14;
    QStringList m_words25;
    QStringListModel *m_completer14Model = nullptr;
    QStringListModel *m_completer25Model = nullptr;
    QCompleter *m_completer14 = nullptr;
    QCompleter *m_completer25 = nullptr;
};

#endif
