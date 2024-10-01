// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WALLETUNLOCKWIDGET_H
#define FEATHER_WALLETUNLOCKWIDGET_H

#include <QWidget>

class Wallet;

namespace Ui {
    class WalletUnlockWidget;
}

class WalletUnlockWidget : public QWidget
{
Q_OBJECT

public:
    explicit WalletUnlockWidget(QWidget *parent, Wallet *wallet = nullptr);
    ~WalletUnlockWidget();

    void setWalletName(const QString &walletName);
    void reset();
    void incorrectPassword();

signals:
    void unlockWallet(const QString &password);
    void closeWallet();

private slots:
    void tryUnlock();

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    QScopedPointer<Ui::WalletUnlockWidget> ui;
    Wallet *m_wallet;
};

#endif //FEATHER_WALLETUNLOCKWIDGET_H
