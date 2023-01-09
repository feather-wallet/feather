// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2023 The Monero Project

#ifndef FEATHER_OPENWALLET_H
#define FEATHER_OPENWALLET_H

#include <QLabel>
#include <QWizardPage>
#include <QWidget>

#include "appcontext.h"
#include "model/WalletKeysFilesModel.h"

namespace Ui {
    class PageOpenWallet;
}

class PageOpenWallet : public QWizardPage
{
    Q_OBJECT

public:
    explicit PageOpenWallet(WalletKeysFilesModel *wallets, QWidget *parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    int nextId() const override;

signals:
    void openWallet(QString path);

private slots:
    void nextPage();

private:
    void updatePath();

    Ui::PageOpenWallet *ui;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WalletKeysFilesProxyModel *m_keysProxy;
    QStandardItemModel *m_model;
    QString m_walletFile;
};

#endif //FEATHER_OPENWALLET_H
