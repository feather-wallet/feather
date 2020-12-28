// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2014-2021, The Monero Project.

#include "libwalletqt/WalletManager.h"
#include "Wallet.h"

//#include "qt/updater.h"
#include "utils/ScopeGuard.h"

class WalletPassphraseListenerImpl : public  Monero::WalletListener, public PassphraseReceiver
{
public:
    WalletPassphraseListenerImpl(WalletManager * mgr): m_mgr(mgr), m_phelper(mgr) {}

    virtual void moneySpent(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    virtual void moneyReceived(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    virtual void unconfirmedMoneyReceived(const std::string &txId, uint64_t amount) override { (void)txId; (void)amount; };
    virtual void newBlock(uint64_t height) override { (void) height; };
    virtual void updated() override {};
    virtual void refreshed(bool success) override {};

    virtual void onPassphraseEntered(const QString &passphrase, bool enter_on_device, bool entry_abort) override
    {
        qDebug() << __FUNCTION__;
        m_phelper.onPassphraseEntered(passphrase, enter_on_device, entry_abort);
    }

//    virtual Monero::optional<std::string> onDevicePassphraseRequest(bool & on_device) override
//    {
//        qDebug() << __FUNCTION__;
//        return m_phelper.onDevicePassphraseRequest(on_device);
//    }
//
//    virtual void onDeviceButtonRequest(uint64_t code) override
//    {
//        qDebug() << __FUNCTION__;
//        emit m_mgr->deviceButtonRequest(code);
//    }
//
//    virtual void onDeviceButtonPressed() override
//    {
//        qDebug() << __FUNCTION__;
//        emit m_mgr->deviceButtonPressed();
//    }

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
    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
    }
    Monero::Wallet * w = m_pimpl->createWallet(path.toStdString(), password.toStdString(),
                                                  language.toStdString(), static_cast<Monero::NetworkType>(nettype), kdfRounds);
    m_currentWallet  = new Wallet(w);
    return m_currentWallet;
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

    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
    }
    qDebug() << QString("%1: opening wallet at %2, nettype = %3 ").arg(__PRETTY_FUNCTION__).arg(qPrintable(path)).arg(nettype);

    Monero::Wallet * w =  m_pimpl->openWallet(path.toStdString(), password.toStdString(), static_cast<Monero::NetworkType>(nettype), kdfRounds, &tmpListener);
    w->setListener(nullptr);

    qDebug() << QString("%1: opened wallet: %2, status: %3").arg(__PRETTY_FUNCTION__).arg(w->address(0, 0).c_str()).arg(w->status());
    m_currentWallet = new Wallet(w);

    // move wallet to the GUI thread. Otherwise it wont be emitting signals
    if (m_currentWallet->thread() != qApp->thread()) {
        m_currentWallet->moveToThread(qApp->thread());
    }

    return m_currentWallet;
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
    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
    }
    Monero::Wallet * w = m_pimpl->recoveryWallet(path.toStdString(), password.toStdString(), seed.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight, kdfRounds, seed_offset.toStdString());
    m_currentWallet = new Wallet(w);
    return m_currentWallet;
}

Wallet *WalletManager::createWalletFromKeys(const QString &path, const QString &language, NetworkType::Type nettype,
                                            const QString &address, const QString &viewkey, const QString &spendkey,
                                            quint64 restoreHeight, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);
    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
        m_currentWallet = NULL;
    }
    Monero::Wallet * w = m_pimpl->createWalletFromKeys(path.toStdString(), "", language.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight,
                                                       address.toStdString(), viewkey.toStdString(), spendkey.toStdString(), kdfRounds);
    m_currentWallet = new Wallet(w);
    return m_currentWallet;
}

Wallet *WalletManager::createDeterministicWalletFromSpendKey(const QString &path, const QString &password, const QString &language, NetworkType::Type nettype,
                                                             const QString &spendkey, quint64 restoreHeight, quint64 kdfRounds)
{
    QMutexLocker locker(&m_mutex);
    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
        m_currentWallet = NULL;
    }
    Monero::Wallet * w = m_pimpl->createDeterministicWalletFromSpendKey(path.toStdString(), "", language.toStdString(), static_cast<Monero::NetworkType>(nettype), restoreHeight,
                                                                        spendkey.toStdString(), kdfRounds);
    m_currentWallet = new Wallet(w);
    return m_currentWallet;
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

    if (m_currentWallet) {
        qDebug() << "Closing open m_currentWallet" << m_currentWallet;
        delete m_currentWallet;
        m_currentWallet = NULL;
    }
    Monero::Wallet * w = m_pimpl->createWalletFromDevice(path.toStdString(), password.toStdString(), static_cast<Monero::NetworkType>(nettype),
                                                         deviceName.toStdString(), restoreHeight, subaddressLookahead.toStdString(), 1, &tmpListener);
    w->setListener(nullptr);

    m_currentWallet = new Wallet(w);

    // move wallet to the GUI thread. Otherwise it wont be emitting signals
    if (m_currentWallet->thread() != qApp->thread()) {
        m_currentWallet->moveToThread(qApp->thread());
    }

    return m_currentWallet;
}


void WalletManager::createWalletFromDeviceAsync(const QString &path, const QString &password, NetworkType::Type nettype,
                                                const QString &deviceName, quint64 restoreHeight, const QString &subaddressLookahead)
{
    m_scheduler.run([this, path, password, nettype, deviceName, restoreHeight, subaddressLookahead] {
        Wallet *wallet = createWalletFromDevice(path, password, nettype, deviceName, restoreHeight, subaddressLookahead);
        emit walletCreated(wallet);
    });
}

QString WalletManager::closeWallet()
{
    QMutexLocker locker(&m_mutex);
    QString result;
    if (m_currentWallet) {
        result = m_currentWallet->address(0, 0);
        delete m_currentWallet;
    } else {
        qCritical() << "Trying to close non existing wallet " << m_currentWallet;
        result = "0";
    }
    return result;
}

// @TODO: fix
//void WalletManager::closeWalletAsync(const QJSValue& callback)
//{
//    m_scheduler.run([this] {
//        return QJSValueList({closeWallet()});
//    }, callback);
//}

bool WalletManager::walletExists(const QString &path) const
{
    return m_pimpl->walletExists(path.toStdString());
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

QString WalletManager::displayAmount(quint64 amount)
{
    return QString::fromStdString(Monero::Wallet::displayAmount(amount));
}

quint64 WalletManager::amountFromString(const QString &amount) const
{
    return Monero::Wallet::amountFromString(amount.toStdString());
}

quint64 WalletManager::amountFromDouble(double amount) const
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

bool WalletManager::keyValid(const QString &key, const QString &address, bool isViewKey,  NetworkType::Type nettype) const
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

double WalletManager::miningHashRate() const
{
    return m_pimpl->miningHashRate();
}

bool WalletManager::isMining() const
{
    {
        QMutexLocker locker(&m_mutex);
        if (m_currentWallet == nullptr || !m_currentWallet->connectionStatus())
        {
            return false;
        }
    }

    return m_pimpl->isMining();
}

void WalletManager::miningStatusAsync()
{
    m_scheduler.run([this] {
        emit miningStatus(isMining());
    });
}

bool WalletManager::startMining(const QString &address, quint32 threads, bool backgroundMining, bool ignoreBattery)
{
    if(threads == 0)
        threads = 1;
    return m_pimpl->startMining(address.toStdString(), threads, backgroundMining, ignoreBattery);
}

bool WalletManager::stopMining()
{
    return m_pimpl->stopMining();
}

bool WalletManager::localDaemonSynced() const
{
    return blockchainHeight() > 1 && blockchainHeight() >= blockchainTargetHeight();
}

bool WalletManager::isDaemonLocal(const QString &daemon_address) const
{
    return daemon_address.isEmpty() ? false : Monero::Utils::isAddressLocal(daemon_address.toStdString());
}

QString WalletManager::resolveOpenAlias(const QString &address) const
{
    bool dnssec_valid = false;
    std::string res = m_pimpl->resolveOpenAlias(address.toStdString(), dnssec_valid);
    res = std::string(dnssec_valid ? "true" : "false") + "|" + res;
    return QString::fromStdString(res);
}
bool WalletManager::parse_uri(const QString &uri, QString &address, QString &payment_id, uint64_t &amount, QString &tx_description, QString &recipient_name, QVector<QString> &unknown_parameters, QString &error) const
{
    QMutexLocker locker(&m_mutex);
    if (m_currentWallet)
        return m_currentWallet->parse_uri(uri, address, payment_id, amount, tx_description, recipient_name, unknown_parameters, error);
    return false;
}

QVariantMap WalletManager::parse_uri_to_object(const QString &uri) const
{
    QString address;
    QString payment_id;
    uint64_t amount = 0;
    QString tx_description;
    QString recipient_name;
    QVector<QString> unknown_parameters;
    QString error;

    QVariantMap result;
    if (this->parse_uri(uri, address, payment_id, amount, tx_description, recipient_name, unknown_parameters, error)) {
        result.insert("address", address);
        result.insert("payment_id", payment_id);
        result.insert("amount", amount > 0 ? displayAmount(amount) : "");
        result.insert("tx_description", tx_description);
        result.insert("recipient_name", recipient_name);
    } else {
        result.insert("error", error);
    }
    
    return result;
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

QString WalletManager::checkUpdates(const QString &software, const QString &subdir) const
{
  qDebug() << "Checking for updates";
  const std::tuple<bool, std::string, std::string, std::string, std::string> result = Monero::WalletManager::checkUpdates(software.toStdString(), subdir.toStdString());
  if (!std::get<0>(result))
    return QString("");
  return QString::fromStdString(std::get<1>(result) + "|" + std::get<2>(result) + "|" + std::get<3>(result) + "|" + std::get<4>(result));
}

bool WalletManager::clearWalletCache(const QString &wallet_path) const
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
