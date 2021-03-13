// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WIZARDRESTORE_H
#define FEATHER_WIZARDRESTORE_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>
#include <QTextEdit>
#include <QCompleter>

#include "utils/textedit.h"
#include "appcontext.h"

namespace Ui {
    class PageWalletRestoreSeed;
}

class PageWalletRestoreSeed : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageWalletRestoreSeed(AppContext *ctx, WizardFields *fields, QWidget *parent = nullptr);
    bool validatePage() override;
    void initializePage() override;
    int nextId() const override;

private:
    struct seedType {
        seedType()
        {
            completer.setModel(&completerModel);
            completer.setModelSorting(QCompleter::CaseInsensitivelySortedModel);
            completer.setCaseSensitivity(Qt::CaseInsensitive);
            completer.setWrapAround(false);
        }

        void setWords(const QStringList &wordlist) {
            this->words = wordlist;
            completerModel.setStringList(words);
        }

        int length;
        QStringList words;
        QStringListModel completerModel;
        QCompleter completer;
    };

    void onSeedTypeToggled();

    AppContext *m_ctx;
    Ui::PageWalletRestoreSeed *ui;
    WizardFields *m_fields;

    seedType m_tevador;
    seedType m_legacy;

    seedType *m_mode;
};

#endif
