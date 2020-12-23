// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef WALLETWIZARD_H
#define WALLETWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QRadioButton>

#include "appcontext.h"
#include "utils/seeds.h"
#include "utils/config.h"

class WalletWizard : public QWizard
{
    Q_OBJECT

public:
    enum Page { Page_Menu, Page_CreateWallet, Page_CreateWalletSeed, Page_OpenWallet, Page_Network, Page_Restore, Page_ViewOnly };

    explicit WalletWizard(AppContext *ctx, WalletWizard::Page startPage = WalletWizard::Page::Page_Menu, QWidget *parent = nullptr);
signals:
    void openWallet(QString path, QString password);

private:
    AppContext *m_ctx;
    WalletKeysFilesModel *m_walletKeysFilesModel;

    void createWallet();
};

#endif // WALLETWIZARD_H
