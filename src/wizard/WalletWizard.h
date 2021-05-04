// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef WALLETWIZARD_H
#define WALLETWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QRadioButton>

#include "appcontext.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/config.h"

enum WizardMode {
    CreateWallet = 0,
    OpenWallet,
    RestoreFromSeed,
    RestoreFromKeys,
    CreateWalletFromDevice
};

struct WizardFields {
    QString walletName;
    QString walletDir;
    QString seed;
    QString seedOffsetPassphrase;
    QString password;
    QString modeText;
    QString address;
    QString secretViewKey;
    QString secretSpendKey;
    WizardMode mode;
    int restoreHeight = 0;
    SeedType seedType;
};

class WalletWizard : public QWizard
{
    Q_OBJECT

public:
    enum Page {
        Page_Menu,
        Page_WalletFile,
        Page_CreateWalletSeed,
        Page_SetPasswordPage,
        Page_OpenWallet,
        Page_Network,
        Page_WalletRestoreSeed,
        Page_WalletRestoreKeys,
        Page_SetRestoreHeight,
        Page_HardwareDevice,
        Page_NetworkTor
    };

    explicit WalletWizard(AppContext *ctx, WalletWizard::Page startPage = WalletWizard::Page::Page_Menu, QWidget *parent = nullptr);

signals:
    void initialNetworkConfigured();
    void skinChanged(const QString &skin);
    void openWallet(QString path, QString password);
    void defaultWalletDirChanged(QString walletDir);

private:
    AppContext *m_ctx;
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields m_wizardFields;

    void createWallet();
};

#endif // WALLETWIZARD_H
