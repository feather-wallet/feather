// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2014-2022 The Monero Project

#include "libwalletqt/WalletManager.h"
#include "Wallet.h"

#include "utils/ScopeGuard.h"

class WalletPassphraseListenerImpl : public  Monero::WalletListener, public PassphraseReceiver
{
public:
    explicit WalletPassphraseListenerImpl(WalletManager * mgr): m_mgr(mgr), m_phelper(mgr) {}

    void moneySpent(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    void moneyReceived(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    void newBlock(uint64_t height) override { (void) height; };
    void updated() override {};
    void refreshed(bool success) override {};

    void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) override
    {
        qDebug() << __FUNCTION__;
        m_phelper.onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }

    Monero::optional<std::string> onDevicePassphraseRequest(bool & on_device) override
    {
        qDebug() << __FUNCTION__;
        return m_phelper.onDevicePassphraseRequest(on_device);
    }

    void onDeviceButtonRequest(uint64_t code) override
    {
        qDebug() << __FUNCTION__;
        emit m_mgr->deviceButtonRequest(code);
    }

    void onDeviceButtonPressed() override
    {
        qDebug() << __FUNCTION__;
        emit m_mgr->deviceButtonPressed();
    }

    void onDeviceError(const std::string &message) override
    {
        qDebug() << __FUNCTION__;
        emit m_mgr->deviceError(QString::fromStdString(message));
    }

private:
    WalletManager * m_mgr;
    PassphraseHelper m_phelper;
};

WalletManager * WalletManager::m_instance = nullptr;

WalletManager *WalletManager::instance()
{
    if (!m_instance) {
        m_instance = new WalletManager;
    }

    return m_instance;
}

Wallet *WalletManager::createWallet(const QString &path, const QString &password,
                                    const QString &language, NetworkType::Type nettype, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);

    Monero::Wallet * w = m_pimpl->createWallet(path.toStdString(), password.toStdString(),
                                                  language.toStdString(), static_cast<Monero::NetworkType>(nettype), kdfRounds);
    return new Wallet(w);
}

Wallet *WalletManager::openWallet(const QString &path, const QString &password, NetworkType::Type nettype, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);
    WalletPassphraseListenerImpl tmpListener(this);
    m_mutex_passphraseReceiver.lock();
    m_passphraseReceiver = &tmpListener;
    m_mutex_passphraseReceiver.unlock();
    const auto cleanup = sg::make_scope_guard([this]() noexcept {
        QMutexLocker passphrase_locker(&m_mutex_passphraseReceiver);
        this->m_passphraseReceiver = nullptr;
    });

    qDebug() << QString("%1: opening wallet at %2, nettype = %3 ").arg(__PRETTY_FUNCTION__).arg(qPrintable(path)).arg(nettype);

    Monero::Wallet * w =  m_pimpl->openWallet(path.toStdString(), password.toStdString(), static_cast<Monero::NetworkType>(nettype), kdfRounds, &tmpListener);
    w->setListener(nullptr);

    qDebug() << QString("%1: opened wallet: %2, status: %3").arg(__PRETTY_FUNCTION__).arg(w->address(0, 0).c_str()).arg(w->status());
    auto wallet = new Wallet(w);

    // move wallet to the GUI thread. Otherwise it wont be emitting signals
    if (wallet->thread() != qApp->thread()) {
        wallet->moveToThread(qApp->thread());
    }

    return wallet;
}

void WalletManager::openWalletAsync(const QString &path, const QString &password, NetworkType::Type nettype, quint64 kdfRounds)
{
    m_scheduler.run([this, path, password, nettype, kdfRounds] {
        emit walletOpened(openWallet(path, password, nettype, kdfRounds));
    });
}


Wallet *WalletManager::recoveryWallet(const QString &path, const QString &password, const QString &seed, const QString &seed_offset, NetworkType::Type nettype, quint64 restoreHeight, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);

    Monero::Wallet * w = m_pimpl->recoveryWallet(path.toStdString(), password.toStdString(), seed.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight, kdfRounds, seed_offset.toStdString());
    return new Wallet(w);
}

Wallet *WalletManager::createWalletFromKeys(const QString &path, const QString &password, const QString &language,
                                            NetworkType::Type nettype, const QString &address, const QString &viewkey,
                                            const QString &spendkey, quint64 restoreHeight, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);
    Monero::Wallet * w = m_pimpl->createWalletFromKeys(path.toStdString(), password.toStdString(), language.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight,
                                                       address.toStdString(), viewkey.toStdString(), spendkey.toStdString(), kdfRounds);
    return new Wallet(w);
}

Wallet *WalletManager::createDeterministicWalletFromSpendKey(const QString &path, const QString &password, const QString &language, NetworkType::Type nettype,
                                                             const QString &spendkey, quint64 restoreHeight, quint64 kdfRounds, const QString &offset_passphrase)
{
    QMutexLocker locker(&m_mutex);
    Monero::Wallet * w = m_pimpl->createDeterministicWalletFromSpendKey(path.toStdString(), password.toStdString(), language.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight,
                                                                        spendkey.toStdString(), kdfRounds, offset_passphrase.toStdString());
    return new Wallet(w);
}

Wallet *WalletManager::createWalletFromDevice(const QString &path, const QString &password, NetworkType::Type nettype,
                                              const QString &deviceName, quint64 restoreHeight, const QString &subaddressLookahead)
{
    QMutexLocker locker(&m_mutex);
    WalletPassphraseListenerImpl tmpListener(this);
    m_mutex_passphraseReceiver.lock();
    m_passphraseReceiver = &tmpListener;
    m_mutex_passphraseReceiver.unlock();
    const auto cleanup = sg::make_scope_guard([this]() noexcept {
        QMutexLocker passphrase_locker(&m_mutex_passphraseReceiver);
        this->m_passphraseReceiver = nullptr;
    });

    Monero::Wallet * w = m_pimpl->createWalletFromDevice(path.toStdString(), password.toStdString(), static_cast<Monero::NetworkType>(nettype),
                                                         deviceName.toStdString(), restoreHeight, subaddressLookahead.toStdString(), 1, &tmpListener);
    w->setListener(nullptr);

    auto wallet = new Wallet(w);

    // move wallet to the GUI thread. Otherwise it wont be emitting signals
    if (wallet->thread() != qApp->thread()) {
        wallet->moveToThread(qApp->thread());
    }

    return wallet;
}

void WalletManager::createWalletFromDeviceAsync(const QString &path, const QString &password, NetworkType::Type nettype,
                                                const QString &deviceName, quint64 restoreHeight, const QString &subaddressLookahead)
{
    m_scheduler.run([this, path, password, nettype, deviceName, restoreHeight, subaddressLookahead] {
        Wallet *wallet = createWalletFromDevice(path, password, nettype, deviceName, restoreHeight, subaddressLookahead);
        emit walletCreated(wallet);
    });
}

bool WalletManager::walletExists(const QString &path) const
{
    return m_pimpl->walletExists(path.toStdString());
}

bool WalletManager::verifyWalletPassword(const QString &keys_file_name, const QString &password, bool no_spend_key, uint64_t kdf_rounds) const
{
    return m_pimpl->verifyWalletPassword(keys_file_name.toStdString(), password.toStdString(), no_spend_key, kdf_rounds);
}

QStringList WalletManager::findWallets(const QString &path)
{
    std::vector<std::string> found_wallets = m_pimpl->findWallets(path.toStdString());
    QStringList result;
    for (const auto &w : found_wallets) {
        result.append(QString::fromStdString(w));
    }
    return result;
}

QString WalletManager::errorString() const
{
    return tr("Unknown error");
}

quint64 WalletManager::maximumAllowedAmount() const
{
    return Monero::Wallet::maximumAllowedAmount();
}

QString WalletManager::maximumAllowedAmountAsString() const
{
    return WalletManager::displayAmount(WalletManager::maximumAllowedAmount());
}

QString WalletManager::displayAmount(quint64 amount, bool trailing_zeroes, int decimals)
{
    auto amountStr = QString::fromStdString(Monero::Wallet::displayAmount(amount));

    if (decimals < 12) {
        int i = amountStr.indexOf(".");
        amountStr.remove(i+decimals+1, 12);
    }

    if (!trailing_zeroes) {
        amountStr.remove(QRegularExpression("0+$"));
        amountStr.remove(QRegularExpression("\\.$"));
    }

    return amountStr;
}

quint64 WalletManager::amountFromString(const QString &amount)
{
    return Monero::Wallet::amountFromString(amount.toStdString());
}

quint64 WalletManager::amountFromDouble(double amount)
{
    return Monero::Wallet::amountFromDouble(amount);
}

bool WalletManager::paymentIdValid(const QString &payment_id) const
{
    return Monero::Wallet::paymentIdValid(payment_id.toStdString());
}

bool WalletManager::addressValid(const QString &address, NetworkType::Type nettype)
{
    return Monero::Wallet::addressValid(address.toStdString(), static_cast<Monero::NetworkType>(nettype));
}

bool WalletManager::keyValid(const QString &key, const QString &address, bool isViewKey,  NetworkType::Type nettype)
{
    std::string error;
    if(!Monero::Wallet::keyValid(key.toStdString(), address.toStdString(), isViewKey, static_cast<Monero::NetworkType>(nettype), error)){
        qDebug() << QString::fromStdString(error);
        return false;
    }
    return true;
}

QString WalletManager::paymentIdFromAddress(const QString &address, NetworkType::Type nettype) const
{
    return QString::fromStdString(Monero::Wallet::paymentIdFromAddress(address.toStdString(), static_cast<Monero::NetworkType>(nettype)));
}

void WalletManager::setDaemonAddressAsync(const QString &address)
{
    m_scheduler.run([this, address] {
        m_pimpl->setDaemonAddress(address.toStdString());
    });
}

bool WalletManager::connected() const
{
    return m_pimpl->connected();
}

quint64 WalletManager::networkDifficulty() const
{
    return m_pimpl->networkDifficulty();
}

quint64 WalletManager::blockchainHeight() const
{
    return m_pimpl->blockchainHeight();
}

quint64 WalletManager::blockchainTargetHeight() const
{
    return m_pimpl->blockchainTargetHeight();
}

bool WalletManager::localDaemonSynced() const
{
    return blockchainHeight() > 1 && blockchainHeight() >= blockchainTargetHeight();
}

bool WalletManager::isDaemonLocal(const QString &daemon_address) const
{
    return daemon_address.isEmpty() ? false : Monero::Utils::isAddressLocal(daemon_address.toStdString());
}

QString WalletManager::resolveOpenAlias(const QString &address, bool &dnssecValid) const
{
    std::string res = m_pimpl->resolveOpenAlias(address.toStdString(), dnssecValid);
    return QString::fromStdString(res);
}

void WalletManager::resolveOpenAliasAsync(const QString &alias) {
    m_scheduler.run([this, alias] {
        bool dnssecValid;
        QString address = this->resolveOpenAlias(alias, dnssecValid);
        emit openAliasResolved(alias, address, dnssecValid);
    });
}

void WalletManager::setLogLevel(int logLevel)
{
    Monero::WalletManagerFactory::setLogLevel(logLevel);
}

void WalletManager::setLogCategories(const QString &categories)
{
    Monero::WalletManagerFactory::setLogCategories(categories.toStdString());
}

QString WalletManager::urlToLocalPath(const QUrl &url) const
{
    return QDir::toNativeSeparators(url.toLocalFile());
}

QUrl WalletManager::localPathToUrl(const QString &path) const
{
    return QUrl::fromLocalFile(path);
}

bool WalletManager::clearWalletCache(const QString &wallet_path)
{
    QString fileName = wallet_path;
    // Make sure wallet file is not .keys
    fileName.replace(".keys","");
    QFile walletCache(fileName);
    QString suffix = ".old_cache";
    QString newFileName = fileName + suffix;

    // create unique file name
    for (int i = 1; QFile::exists(newFileName); i++) {
       newFileName = QString("%1%2.%3").arg(fileName).arg(suffix).arg(i);
    }

    return walletCache.rename(newFileName);
}

WalletManager::WalletManager(QObject *parent)
    : QObject(parent)
    , m_passphraseReceiver(nullptr)
    , m_scheduler(this)
{
    m_pimpl =  Monero::WalletManagerFactory::getWalletManager();
}

WalletManager::~WalletManager()
{
    m_scheduler.shutdownWaitForFinished();
}

void WalletManager::onWalletPassphraseNeeded(bool on_device)
{
    emit this->walletPassphraseNeeded(on_device);
}

void WalletManager::onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort)
{
    QMutexLocker locker(&m_mutex_passphraseReceiver);
    if (m_passphraseReceiver != nullptr)
    {
        m_passphraseReceiver->onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }
}

QString WalletManager::proxyAddress() const
{
    QMutexLocker locker(&m_proxyMutex);
    return m_proxyAddress;
}

void WalletManager::setProxyAddress(QString address)
{
    m_scheduler.run([this, address] {
        {
            QMutexLocker locker(&m_proxyMutex);

            if (!m_pimpl->setProxy(address.toStdString()))
            {
                qCritical() << "Failed to set proxy address" << address;
            }

            m_proxyAddress = std::move(address);
        }
        emit proxyAddressChanged();
    });
}