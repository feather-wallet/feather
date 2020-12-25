// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_CREATEWALLETSEED_H
#define FEATHER_CREATEWALLETSEED_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "utils/utils.h"
#include "appcontext.h"

namespace Ui {
    class CreateWalletSeedPage;
}

class CreateWalletSeedPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit CreateWalletSeedPage(AppContext *ctx, QWidget *parent = nullptr);
    bool validatePage() override;
    int nextId() const override;

public slots:
    void displaySeed(const QString &seed);

private:
    void seedRoulette(int count);

signals:
    void createWallet();

private:
    AppContext *m_ctx;
    QLabel *topLabel;
    Ui::CreateWalletSeedPage *ui;

    QString m_mnemonic;
    int m_restoreHeight;

    bool m_roulette = false;
    int m_rouletteSpin = 15;
};

#endif //FEATHER_CREATEWALLETSEED_H
