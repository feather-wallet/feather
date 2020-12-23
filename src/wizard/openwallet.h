// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020, The Monero Project.

#ifndef FEATHER_OPENWALLET_H
#define FEATHER_OPENWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"
#include "utils/keysfiles.h"

namespace Ui {
    class OpenWalletPage;
}

class OpenWalletPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit OpenWalletPage(AppContext *ctx, WalletKeysFilesModel *wallets, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void openWallet(QString path);

private:
    void updatePath();

    AppContext *m_ctx;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WalletKeysFilesProxyModel *m_keysProxy;
    Ui::OpenWalletPage *ui;
    QStandardItemModel *m_model;
};

#endif //FEATHER_OPENWALLET_H
