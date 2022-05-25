// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#ifndef FEATHER_WALLETWIZARD_H
#define FEATHER_WALLETWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QRadioButton>

#include "appcontext.h"
#include "model/WalletKeysFilesModel.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/config.h"
#include "constants.h"

enum WizardMode {
    CreateWallet = 0,
    OpenWallet,
    RestoreFromSeed,
    RestoreFromKeys,
    CreateWalletFromDevice
};

enum DeviceType {
    LEDGER = 0,
    TREZOR
};

struct WizardFields {
    QString walletName;
    QString walletDir;
    Seed seed;
    bool seedOffsetPassphraseEnabled = false;
    QString seedOffsetPassphrase;
    QString seedLanguage = constants::seedLanguage;
    QString password;
    QString modeText;
    QString address;
    QString secretViewKey;
    QString secretSpendKey;
    WizardMode mode;
    int restoreHeight = 0;
    Seed::Type seedType;
    DeviceType deviceType;

    WizardFields(): deviceType(DeviceType::LEDGER), mode(WizardMode::CreateWallet),
    seedType(Seed::POLYSEED), seedOffsetPassphraseEnabled(false), restoreHeight(0) {}
};

class WalletWizard : public QWizard
{
    Q_OBJECT

public:
    enum Page {
        Page_Menu,
        Page_WalletFile,
        Page_CreateWalletSeed,
        Page_SetSeedPassphrase,
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
    void resetFields();

signals:
    void initialNetworkConfigured();
    void skinChanged(const QString &skin);
    void openWallet(QString path, QString password);
    void defaultWalletDirChanged(QString walletDir);

    void createWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight);
    void createWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, bool deterministic = false);
    void createWallet(Seed seed, const QString &path, const QString &password, const QString &seedLanguage, const QString &seedOffset = "");

private slots:
    void onCreateWallet();

private:
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields m_wizardFields;
};

#endif // FEATHER_WALLETWIZARD_H
