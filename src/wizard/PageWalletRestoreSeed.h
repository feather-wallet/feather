// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WIZARDRESTORE_H
#define FEATHER_WIZARDRESTORE_H

#include <QWizardPage>
#include <QCompleter>
#include <QStringListModel>

#include "components.h"

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
            completer.setCompletionMode(QCompleter::UnfilteredPopupCompletion);
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
        SpaceCompleter completer;
        Seed::Type type;
    };

    void onSeedTypeToggled();
    void onSeedLanguageChanged(const QString &language);
    void onOptionsClicked();

    Ui::PageWalletRestoreSeed *ui;
    WizardFields *m_fields;

    seedType m_polyseed;
    seedType m_tevador;
    seedType m_legacy;

    seedType *m_mode;

    QMap<QString, QStringList> m_wordlists;
};

#endif
