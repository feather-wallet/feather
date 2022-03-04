// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#ifndef WALLETMANAGER_H
#define WALLETMANAGER_H

#include <QVariant>
#include <QObject>
#include <QUrl>
#include <wallet/api/wallet2_api.h>
#include <QMutex>
#include <QPointer>
#include <QWaitCondition>
#include "utils/scheduler.h"
#include "utils/networktype.h"
#include "PassphraseHelper.h"

class Wallet;
namespace Monero {
    struct WalletManager;
}

class WalletManager : public QObject, public PassprasePrompter
{
    Q_OBJECT

public:
    enum LogLevel {
        LogLevel_Silent = Monero::WalletManagerFactory::LogLevel_Silent,
        LogLevel_0 = Monero::WalletManagerFactory::LogLevel_0,
        LogLevel_1 = Monero::WalletManagerFactory::LogLevel_1,
        LogLevel_2 = Monero::WalletManagerFactory::LogLevel_2,
        LogLevel_3 = Monero::WalletManagerFactory::LogLevel_3,
        LogLevel_4 = Monero::WalletManagerFactory::LogLevel_4,
        LogLevel_Min = Monero::WalletManagerFactory::LogLevel_Min,
        LogLevel_Max = Monero::WalletManagerFactory::LogLevel_Max,
    };

    static WalletManager * instance();
    // wizard: createWallet path;
    Wallet * createWallet(const QString &path, const QString &password,
                                      const QString &language, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    /*!
     * \brief openWallet - opens wallet by given path
     * \param path       - wallet filename
     * \param password   - wallet password. Empty string in wallet isn't password protected
     * \param nettype    - type of network the wallet is running on
     * \return wallet object pointer
     */
    Wallet * openWallet(const QString &path, const QString &password, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    /*!
     * \brief openWalletAsync - asynchronous version of "openWallet". Returns immediately. "walletOpened" signal
     *                          emitted when wallet opened;
     */
    void openWalletAsync(const QString &path, const QString &password, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    // wizard: recoveryWallet path; hint: internally it recorvers wallet and set password = ""
    Wallet * recoveryWallet(const QString &path, const QString &password, const QString &seed, const QString &seed_offset,
                                       NetworkType::Type nettype = NetworkType::MAINNET, quint64 restoreHeight = 0, quint64 kdfRounds = 1);

    Wallet * createWalletFromKeys(const QString &path,
                                              const QString &password,
                                              const QString &language,
                                              NetworkType::Type nettype,
                                              const QString &address,
                                              const QString &viewkey,
                                              const QString &spendkey = "",
                                              quint64 restoreHeight = 0,
                                              quint64 kdfRounds = 1);

    Wallet * createDeterministicWalletFromSpendKey(const QString &path,
                                                               const QString &password,
                                                               const QString &language,
                                                               NetworkType::Type nettype,
                                                               const QString &spendkey,
                                                               quint64 restoreHeight,
                                                               quint64 kdfRounds,
                                                               const QString &offset_passphrase = "");

    Wallet * createWalletFromDevice(const QString &path,
                                                const QString &password,
                                                NetworkType::Type nettype,
                                                const QString &deviceName,
                                                quint64 restoreHeight = 0,
                                                const QString &subaddressLookahead = "");

    void createWalletFromDeviceAsync(const QString &path,
                                                const QString &password,
                                                NetworkType::Type nettype,
                                                const QString &deviceName,
                                                quint64 restoreHeight = 0,
                                                const QString &subaddressLookahead = "");


    //! checks is given filename is a wallet;
    bool walletExists(const QString &path) const;

    //! verify wallet password
    bool verifyWalletPassword(const QString &keys_file_name, const QString &password, bool no_spend_key, uint64_t kdf_rounds = 1) const;

    //! returns list with wallet's filenames, if found by given path
    QStringList findWallets(const QString &path);

    //! since we can't call static method from QML, move it to this class
    static QString displayAmount(quint64 amount, bool trailing_zeroes = true, int decimals = 12);
    static quint64 amountFromString(const QString &amount);
    static quint64 amountFromDouble(double amount);
    quint64 maximumAllowedAmount() const;

    bool paymentIdValid(const QString &payment_id) const;
    static bool addressValid(const QString &address, NetworkType::Type nettype);
    static bool keyValid(const QString &key, const QString &address, bool isViewKey, NetworkType::Type nettype);

    QString paymentIdFromAddress(const QString &address, NetworkType::Type nettype) const;

    bool isDaemonLocal(const QString &daemon_address) const;

    void setLogLevel(int logLevel);
    void setLogCategories(const QString &categories);

    QString resolveOpenAlias(const QString &address, bool &dnssecValid) const;
    void resolveOpenAliasAsync(const QString &address);

    // clear/rename wallet cache
    static bool clearWalletCache(const QString &fileName);

    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort=false);
    virtual void onWalletPassphraseNeeded(bool on_device) override;

signals:
    void walletOpened(Wallet * wallet);
    void walletCreated(Wallet * wallet);
    void walletPassphraseNeeded(bool onDevice);
    void deviceButtonRequest(quint64 buttonCode);
    void deviceButtonPressed();
    void deviceError(const QString &message);
    void openAliasResolved(const QString &alias, const QString &address, bool dnssecValid);

private:
    friend class WalletPassphraseListenerImpl;

    explicit WalletManager(QObject *parent = nullptr);
    ~WalletManager() override;

    static WalletManager *m_instance;
    Monero::WalletManager *m_pimpl;
    mutable QMutex m_mutex;
    PassphraseReceiver *m_passphraseReceiver;
    QMutex m_mutex_passphraseReceiver;
    FutureScheduler m_scheduler;
};

#endif // WALLETMANAGER_H
