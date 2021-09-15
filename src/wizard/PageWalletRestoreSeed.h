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
    explicit PageWalletRestoreSeed(WizardFields *fields, QWidget *parent = nullptr);
    bool validatePage() override;
    void initializePage() override;
    int nextId() const override;

private:
    struct seedType {
        seedType()
        {
            completer.setModel(&completerModel);
            completer.setModelSorting(QCompleter::CaseSensitivelySortedModel);
            completer.setCaseSensitivity(Qt::CaseSensitive);
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
    void onSeedLanguageChanged(const QString &language);

    Ui::PageWalletRestoreSeed *ui;
    WizardFields *m_fields;

    seedType m_tevador;
    seedType m_legacy;

    seedType *m_mode;

    QMap<QString, QStringList> m_wordlists;
};

#endif
