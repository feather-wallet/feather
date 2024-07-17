// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#ifndef FEATHER_WALLETWIZARD_H
#define FEATHER_WALLETWIZARD_H

#include <QWizard>
#include <QLabel>
#include <QRadioButton>

#include "model/WalletKeysFilesModel.h"
#include "utils/RestoreHeightLookup.h"
#include "utils/config.h"
#include "utils/Seed.h"
#include "constants.h"

enum WizardMode {
    CreateWallet = 0,
    OpenWallet,
    RestoreFromSeed,
    RestoreFromKeys,
    CreateWalletFromDevice,
    CreateMultisig,
    RestoreMultisig
};

enum DeviceType {
    LEDGER = 0,
    TREZOR
};

enum SignerConfig {
    AUTOMATIC = 0,
    SEMI_AUTOMATIC,
    MANUAL
};

struct MMSSigner {
    quint32 index = 0;
    QString label;
    QString address;
};

struct WizardFields {
    QString walletName;
    QString walletDir;
    Seed seed;
    bool showSetSeedPassphrasePage = false;
    bool showSetRestoreHeightPage = false;
    bool showSetSubaddressLookaheadPage = false;
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
    QString subaddressLookahead;
    bool multisigInitiator = false;
    QString multisigSetupKey;
    quint32 multisigThreshold = 0;
    quint32 multisigSigners = 0;
    bool multisigAutomaticSetup = true;
    QString multisigUsername;
    QString multisigService;
    QString multisigChannel;
    QString multisigSeed;
    QString multisigMMSRecovery;
    Wallet *wallet = nullptr;

    void clearFields() {
        showSetSeedPassphrasePage = false;
        showSetRestoreHeightPage = false;
        showSetSubaddressLookaheadPage = false;
        seedOffsetPassphrase = "";
        password = "";
        address = "";
        secretViewKey = "";
        secretSpendKey = "";
        restoreHeight = 0;
        subaddressLookahead = "";
        wallet = nullptr;
    }

    WizardFields(): deviceType(DeviceType::LEDGER), mode(WizardMode::CreateWallet),
                    seedType(Seed::POLYSEED), restoreHeight(0) {}
};

class WalletWizard : public QWizard
{
    Q_OBJECT

public:
    enum Page {
        Page_Menu = 0,
        Page_WalletFile,
        Page_CreateWalletSeed,
        Page_SetSeedPassphrase,
        Page_SetPasswordPage,
        Page_SetSubaddressLookahead,
        Page_OpenWallet,
        Page_Network,
        Page_Recover,
        Page_WalletRestoreSeed,
        Page_WalletRestoreKeys,
        Page_SetRestoreHeight,
        Page_HardwareDevice,
        Page_NetworkProxy,
        Page_NetworkWebsocket,
        Page_Plugins,
        Page_MultisigExperimentalWarning,
        Page_MultisigCreateSetupKey,
        Page_MultisigParticipants,
        Page_MultisigOwnAddress,
        Page_MultisigSignerInfo,
        Page_MultisigSetupDebug,
        Page_MultisigSeed,
        Page_MultisigEnterSetupKey,
        Page_MultisigEnterChannel,
        Page_MultisigSignerConfig,
        Page_MultisigShowSetupKey,
        Page_MultisigEnterName,
        Page_MultisigSetupWallet,
        Page_MultisigVerifyAddress,
        Page_MultisigRestoreSeed,
        Page_MultisigMMSRecoveryInfo,
        Page_MultisigRestoreMMSRecoveryInfo,
        Page_KeyType
    };

    explicit WalletWizard(QWidget *parent = nullptr);
    ~WalletWizard() override;

    void resetFields();
    void setWallet(Wallet* wallet);

signals:
    void initialNetworkConfigured();
    void showSettings();
    void openWallet(QString path, QString password);
    void showWallet(Wallet *wallet);

    void createWalletFromDevice(const QString &path, const QString &password, const QString &deviceName, int restoreHeight, const QString &subaddressLookahead);
    void createWalletFromKeys(const QString &path, const QString &password, const QString &address, const QString &viewkey, const QString &spendkey, quint64 restoreHeight, const QString subaddressLookahead = "");
    void createWallet(Seed seed, const QString &path, const QString &password, const QString &seedLanguage, const QString &seedOffset, const QString &subaddressLookahead, bool newWallet, bool giveToWizard);
    void restoreMultisigWallet(const QString &path, const QString &password, const QString &multisigSeed, const QString &mmsRecovery, quint64 restoreHeight, const QString &subaddressLookahead);

private slots:
    void onCreateWallet();
    QString helpPage();
    void showHelp();

private:
    WalletKeysFilesModel *m_walletKeysFilesModel;
    WizardFields m_wizardFields;
};

#endif // FEATHER_WALLETWIZARD_H
