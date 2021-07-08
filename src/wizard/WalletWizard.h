// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2020-2021, The Monero Project.

#ifndef FEATHER_WALLETWIZARD_H
#define FEATHER_WALLETWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QRadioButton>

#include "appcontext.h"
#include "model/WalletKeysFilesModel.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/config.h"

enum WizardMode {
    CreateWallet = 0,
    OpenWallet,
    RestoreFromSeed,
    RestoreFromKeys,
    CreateWalletFromDevice
};

enum DeviceType {
    LEDGER_NANO_S = 0,
    LEDGER_NANO_X,
    TREZOR_MODEL_T
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
    DeviceType deviceType;
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

    explicit WalletWizard(QWidget *parent = nullptr);

signals:
    void initialNetworkConfigured();
    void skinChanged(const QString &skin);
    void openWallet(QString path, QString password);
    void defaultWalletDirChanged(QString walletDir);

    void createWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight);
    void createWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, bool deterministic = false);
    void createWallet(FeatherSeed seed, const QString &path, const QString &password, const QString &seedOffset = "");

private slots:
    void onCreateWallet();

private:
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields m_wizardFields;
};

#endif // FEATHER_WALLETWIZARD_H
