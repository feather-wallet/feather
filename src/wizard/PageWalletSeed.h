// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_CREATEWALLETSEED_H
#define FEATHER_CREATEWALLETSEED_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "utils/Utils.h"

namespace Ui {
    class PageWalletSeed;
}

class PageWalletSeed : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageWalletSeed(WizardFields *fields, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;
    int nextId() const override;

public slots:
    void displaySeed(const QString &seed);

private:
    void seedRoulette(int count);
    void generateSeed();

signals:
    void createWallet();

private:
    Ui::PageWalletSeed *ui;

    WizardFields *m_fields;

    int m_restoreHeight;

    bool m_seedError = false;
    bool m_roulette = false;
    int m_rouletteSpin = 15;
    Seed m_seed;
};

#endif //FEATHER_CREATEWALLETSEED_H
