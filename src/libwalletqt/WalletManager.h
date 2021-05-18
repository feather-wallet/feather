// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

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
    Q_PROPERTY(bool connected READ connected)

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
    Q_INVOKABLE Wallet * createWallet(const QString &path, const QString &password,
                                      const QString &language, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    /*!
     * \brief openWallet - opens wallet by given path
     * \param path       - wallet filename
     * \param password   - wallet password. Empty string in wallet isn't password protected
     * \param nettype    - type of network the wallet is running on
     * \return wallet object pointer
     */
    Q_INVOKABLE Wallet * openWallet(const QString &path, const QString &password, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    /*!
     * \brief openWalletAsync - asynchronous version of "openWallet". Returns immediately. "walletOpened" signal
     *                          emitted when wallet opened;
     */
    Q_INVOKABLE void openWalletAsync(const QString &path, const QString &password, NetworkType::Type nettype = NetworkType::MAINNET, quint64 kdfRounds = 1);

    // wizard: recoveryWallet path; hint: internally it recorvers wallet and set password = ""
    Q_INVOKABLE Wallet * recoveryWallet(const QString &path, const QString &password, const QString &seed, const QString &seed_offset,
                                       NetworkType::Type nettype = NetworkType::MAINNET, quint64 restoreHeight = 0, quint64 kdfRounds = 1);

    Q_INVOKABLE Wallet * createWalletFromKeys(const QString &path,
                                              const QString &password,
                                              const QString &language,
                                              NetworkType::Type nettype,
                                              const QString &address,
                                              const QString &viewkey,
                                              const QString &spendkey = "",
                                              quint64 restoreHeight = 0,
                                              quint64 kdfRounds = 1);

    Q_INVOKABLE Wallet * createDeterministicWalletFromSpendKey(const QString &path,
                                                               const QString &password,
                                                               const QString &language,
                                                               NetworkType::Type nettype,
                                                               const QString &spendkey,
                                                               quint64 restoreHeight,
                                                               quint64 kdfRounds,
                                                               const QString &offset_passphrase = "");

    Q_INVOKABLE Wallet * createWalletFromDevice(const QString &path,
                                                const QString &password,
                                                NetworkType::Type nettype,
                                                const QString &deviceName,
                                                quint64 restoreHeight = 0,
                                                const QString &subaddressLookahead = "");

    Q_INVOKABLE void createWalletFromDeviceAsync(const QString &path,
                                                const QString &password,
                                                NetworkType::Type nettype,
                                                const QString &deviceName,
                                                quint64 restoreHeight = 0,
                                                const QString &subaddressLookahead = "");


    //! checks is given filename is a wallet;
    Q_INVOKABLE bool walletExists(const QString &path) const;

    //! verify wallet password
    Q_INVOKABLE bool verifyWalletPassword(const QString &keys_file_name, const QString &password, bool no_spend_key, uint64_t kdf_rounds = 1) const;

    //! returns list with wallet's filenames, if found by given path
    Q_INVOKABLE QStringList findWallets(const QString &path);

    //! returns error description in human language
    Q_INVOKABLE QString errorString() const;

    //! since we can't call static method from QML, move it to this class
    Q_INVOKABLE static QString displayAmount(quint64 amount);
    Q_INVOKABLE static quint64 amountFromString(const QString &amount);
    Q_INVOKABLE static quint64 amountFromDouble(double amount);
    Q_INVOKABLE quint64 maximumAllowedAmount() const;

    // QML JS engine doesn't support unsigned integers
    Q_INVOKABLE QString maximumAllowedAmountAsString() const;

    Q_INVOKABLE bool paymentIdValid(const QString &payment_id) const;
    Q_INVOKABLE static bool addressValid(const QString &address, NetworkType::Type nettype);
    Q_INVOKABLE static bool keyValid(const QString &key, const QString &address, bool isViewKey, NetworkType::Type nettype);

    Q_INVOKABLE QString paymentIdFromAddress(const QString &address, NetworkType::Type nettype) const;

    Q_INVOKABLE void setDaemonAddressAsync(const QString &address);
    Q_INVOKABLE bool connected() const;
    Q_INVOKABLE quint64 networkDifficulty() const;
    Q_INVOKABLE quint64 blockchainHeight() const;
    Q_INVOKABLE quint64 blockchainTargetHeight() const;
    Q_INVOKABLE bool localDaemonSynced() const;
    Q_INVOKABLE bool isDaemonLocal(const QString &daemon_address) const;

    // QML missing such functionality, implementing these helpers here
    Q_INVOKABLE QString urlToLocalPath(const QUrl &url) const;
    Q_INVOKABLE QUrl localPathToUrl(const QString &path) const;

    Q_INVOKABLE void setLogLevel(int logLevel);
    Q_INVOKABLE void setLogCategories(const QString &categories);

    Q_INVOKABLE quint64 add(quint64 x, quint64 y) const { return x + y; }
    Q_INVOKABLE quint64 sub(quint64 x, quint64 y) const { return x - y; }
    Q_INVOKABLE qint64 addi(qint64 x, qint64 y) const { return x + y; }
    Q_INVOKABLE qint64 subi(qint64 x, qint64 y) const { return x - y; }

    Q_INVOKABLE QString resolveOpenAlias(const QString &address) const;
    Q_INVOKABLE bool parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const;
    Q_INVOKABLE QVariantMap parse_uri_to_object(const QString &uri) const;

    // clear/rename wallet cache
    Q_INVOKABLE static bool clearWalletCache(const QString &fileName);

    Q_INVOKABLE void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort=false);
    virtual void onWalletPassphraseNeeded(bool on_device) override;

    QString proxyAddress() const;
    void setProxyAddress(QString address);

signals:
    void walletOpened(Wallet * wallet);
    void walletCreated(Wallet * wallet);
    void walletPassphraseNeeded(bool onDevice);
    void deviceButtonRequest(quint64 buttonCode);
    void deviceButtonPressed();
    void deviceError(const QString &message);
    void miningStatus(bool isMining) const;
    void proxyAddressChanged() const;

public slots:
private:
    friend class WalletPassphraseListenerImpl;

    explicit WalletManager(QObject *parent = nullptr);
    ~WalletManager() override;

    static WalletManager *m_instance;
    Monero::WalletManager *m_pimpl;
    mutable QMutex m_mutex;
    PassphraseReceiver *m_passphraseReceiver;
    QMutex m_mutex_passphraseReceiver;
    QString m_proxyAddress;
    mutable QMutex m_proxyMutex;
    FutureScheduler m_scheduler;
};

#endif // WALLETMANAGER_H
