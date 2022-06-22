// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Icons.h"
#include "utils/WebsocketNotifier.h"
#include "utils/NetworkManager.h"

Settings::Settings(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::Settings)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));

    ui->tabWidget->setTabVisible(5, false);
    ui->tabWidget->setCurrentIndex(config()->get(Config::lastSettingsPage).toInt());
    connect(ui->tabWidget, &QTabWidget::currentChanged, [](int index){
            config()->set(Config::lastSettingsPage, index);
    });

    this->setupGeneralTab();
    this->setupPrivacyTab();
    this->setupNodeTab();
    this->setupPathsTab();
    this->setupLinksTab();

    connect(ui->closeButton, &QDialogButtonBox::accepted, this, &Settings::close);

    this->adjustSize();
}

void Settings::setupGeneralTab() {
    // [Preferred fiat currency]
    QStringList fiatCurrencies;
    for (int index = 0; index < ui->comboBox_fiatCurrency->count(); index++) {
        fiatCurrencies << ui->comboBox_fiatCurrency->itemText(index);
    }

    auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if (!preferredFiatCurrency.isEmpty() && fiatCurrencies.contains(preferredFiatCurrency)) {
        ui->comboBox_fiatCurrency->setCurrentText(preferredFiatCurrency);
    }

    connect(ui->comboBox_fiatCurrency, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::fiatCurrencySelected);

    // [Appearance]
    this->setupSkinCombobox();
    auto settingsSkin = config()->get(Config::skin).toString();
    if (m_skins.contains(settingsSkin)) {
        ui->comboBox_skin->setCurrentIndex(m_skins.indexOf(settingsSkin));
    }

    connect(ui->comboBox_skin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_skinChanged);

    // [Amount precision]
    for (int i = 0; i <= 12; i++) {
        ui->comboBox_amountPrecision->addItem(QString::number(i));
    }
    ui->comboBox_amountPrecision->setCurrentIndex(config()->get(Config::amountPrecision).toInt());

    connect(ui->comboBox_amountPrecision, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_amountPrecisionChanged);

    // [Date format]
    QDateTime now = QDateTime::currentDateTime();
    for (const auto & format : m_dateFormats) {
        ui->comboBox_dateFormat->addItem(now.toString(format));
    }
    QString dateFormatSetting = config()->get(Config::dateFormat).toString();
    if (m_dateFormats.contains(dateFormatSetting)) {
        ui->comboBox_dateFormat->setCurrentIndex(m_dateFormats.indexOf(dateFormatSetting));
    }

    connect(ui->comboBox_dateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_dateFormatChanged);

    // [Time format]
    for (const auto & format : m_timeFormats) {
        ui->comboBox_timeFormat->addItem(now.toString(format));
    }
    QString timeFormatSetting = config()->get(Config::timeFormat).toString();
    if (m_timeFormats.contains(timeFormatSetting)) {
        ui->comboBox_timeFormat->setCurrentIndex(m_timeFormats.indexOf(timeFormatSetting));
    }

    connect(ui->comboBox_timeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_timeFormatChanged);

    // [Balance display]
    ui->comboBox_balanceDisplay->setCurrentIndex(config()->get(Config::balanceDisplay).toInt());
    connect(ui->comboBox_balanceDisplay, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_balanceDisplayChanged);
}

void Settings::setupPrivacyTab() {
    // [Multibroadcast outgoing transactions]
    ui->checkBox_multiBroadcast->setChecked(config()->get(Config::multiBroadcast).toBool());
    connect(ui->checkBox_multiBroadcast, &QCheckBox::toggled, [](bool toggled){
        config()->set(Config::multiBroadcast, toggled);
    });
    connect(ui->btn_multiBroadcast, &QPushButton::clicked, [this]{
        QMessageBox::information(this, "Multibroadcasting", "Multibroadcasting relays outgoing transactions to all nodes in your selected node list. This may improve transaction relay speed and reduces the chance of your transaction failing.");
    });

    // [Warn before opening external link]
    ui->checkBox_externalLink->setChecked(config()->get(Config::warnOnExternalLink).toBool());
    connect(ui->checkBox_externalLink, &QCheckBox::clicked, this, &Settings::checkboxExternalLinkWarn);

    // [Hide balance]
    ui->checkBox_hideBalance->setChecked(config()->get(Config::hideBalance).toBool());
    connect(ui->checkBox_hideBalance, &QCheckBox::toggled, [this](bool toggled){
        config()->set(Config::hideBalance, toggled);
        m_ctx->updateBalance();
    });

    // [Disable websocket]
    ui->checkBox_enableWebsocket->setChecked(!config()->get(Config::disableWebsocket).toBool());
    connect(ui->checkBox_enableWebsocket, &QCheckBox::toggled, [this](bool checked){
        config()->set(Config::disableWebsocket, !checked);
        this->enableWebsocket(checked);
    });
    connect(ui->btn_enableWebsocket, &QPushButton::clicked, [this]{
        QMessageBox::information(this, "Obtain third-party information", "Feather can connect to an onion service hosted by the Feather developers to obtain pricing information, a curated list of remote nodes, Home feeds, the latest version of Feather Wallet and more.\n\n"
                                                                         "This service is only used to fetch information and can only be reached over Tor. The wallet does not send information about its state or your transactions to the server. It is not used for any telemetry or crash reports.\n\n"
                                                                         "If you opt to disable this connection some wallet functionality will be disabled. You can re-enable it at any time.");
    });

    // [Do not write log files to disk]
    ui->checkBox_disableLogging->setChecked(config()->get(Config::disableLogging).toBool());
    connect(ui->checkBox_disableLogging, &QCheckBox::toggled, [this](bool toggled){
        config()->set(Config::disableLogging, toggled);
        WalletManager::instance()->setLogLevel(toggled ? -1 : config()->get(Config::logLevel).toInt());
    });

    // [Lock wallet on inactivity]
    ui->checkBox_inactivityLockTimeout->setChecked(config()->get(Config::inactivityLockEnabled).toBool());
    ui->spinBox_inactivityLockTimeout->setValue(config()->get(Config::inactivityLockTimeout).toInt());
    connect(ui->checkBox_inactivityLockTimeout, &QCheckBox::toggled, [](bool toggled){
        config()->set(Config::inactivityLockEnabled, toggled);
    });
    connect(ui->spinBox_inactivityLockTimeout, QOverload<int>::of(&QSpinBox::valueChanged), [](int value){
        config()->set(Config::inactivityLockTimeout, value);
    });

    // [Offline mode]
    ui->checkBox_offlineMode->setChecked(config()->get(Config::offlineMode).toBool());
    connect(ui->checkBox_offlineMode, &QCheckBox::toggled, [this](bool checked){
        config()->set(Config::offlineMode, checked);
        m_ctx->wallet->setOffline(checked);
        this->enableWebsocket(!checked);
    });
}

void Settings::setupNodeTab() {
    ui->nodeWidget->setupUI(m_ctx);
    connect(ui->nodeWidget, &NodeWidget::nodeSourceChanged, m_ctx->nodes, &Nodes::onNodeSourceChanged);
    connect(ui->nodeWidget, &NodeWidget::connectToNode, m_ctx->nodes, QOverload<const FeatherNode&>::of(&Nodes::connectToNode));
}

void Settings::setupPathsTab() {
    ui->lineEdit_defaultWalletDir->setText(config()->get(Config::walletDirectory).toString());
    ui->lineEdit_configDir->setText(Config::defaultConfigDir().path());
    ui->lineEdit_applicationDir->setText(Utils::applicationPath());

    connect(ui->btn_browseDefaultWalletDir, &QPushButton::clicked, [this]{
        QString walletDirOld = config()->get(Config::walletDirectory).toString();
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", walletDirOld, QFileDialog::ShowDirsOnly);
        if (walletDir.isEmpty())
            return;
        config()->set(Config::walletDirectory, walletDir);
        ui->lineEdit_defaultWalletDir->setText(walletDir);
    });
}

void Settings::setupLinksTab() {
    // [Block explorer]
    ui->combo_blockExplorer->setCurrentIndex(ui->combo_blockExplorer->findText(config()->get(Config::blockExplorer).toString()));
    connect(ui->combo_blockExplorer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_blockExplorerChanged);

    // [Reddit frontend]
    ui->combo_redditFrontend->setCurrentIndex(ui->combo_redditFrontend->findText(config()->get(Config::redditFrontend).toString()));
    connect(ui->combo_redditFrontend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_redditFrontendChanged);

    // [LocalMonero frontend]
    this->setupLocalMoneroFrontendCombobox();
    connect(ui->combo_localMoneroFrontend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_localMoneroFrontendChanged);
}

void Settings::fiatCurrencySelected(int index) {
    QString selection = ui->comboBox_fiatCurrency->itemText(index);
    config()->set(Config::preferredFiatCurrency, selection);
    emit preferredFiatCurrencyChanged(selection);
}

void Settings::comboBox_skinChanged(int pos) {
    emit skinChanged(m_skins.at(pos));
}

void Settings::comboBox_blockExplorerChanged(int pos) {
    QString blockExplorer = ui->combo_blockExplorer->currentText();
    config()->set(Config::blockExplorer, blockExplorer);
    emit blockExplorerChanged(blockExplorer);
}

void Settings::comboBox_redditFrontendChanged(int pos) {
    QString redditFrontend = ui->combo_redditFrontend->currentText();
    config()->set(Config::redditFrontend, redditFrontend);
}

void Settings::comboBox_localMoneroFrontendChanged(int pos) {
    QString localMoneroFrontend = ui->combo_localMoneroFrontend->currentData().toString();
    config()->set(Config::localMoneroFrontend, localMoneroFrontend);
}

void Settings::comboBox_amountPrecisionChanged(int pos) {
    config()->set(Config::amountPrecision, pos);
    m_ctx->updateBalance();
    emit amountPrecisionChanged(pos);
}

void Settings::comboBox_dateFormatChanged(int pos) {
    config()->set(Config::dateFormat, m_dateFormats.at(pos));
}

void Settings::comboBox_timeFormatChanged(int pos) {
    config()->set(Config::timeFormat, m_timeFormats.at(pos));
}

void Settings::comboBox_balanceDisplayChanged(int pos) {
    config()->set(Config::balanceDisplay, pos);
    m_ctx->updateBalance();
}

void Settings::checkboxExternalLinkWarn() {
    bool state = ui->checkBox_externalLink->isChecked();
    config()->set(Config::warnOnExternalLink, state);
}

void Settings::setupSkinCombobox() {
#if defined(Q_OS_WIN)
    m_skins.removeOne("Breeze/Dark");
    m_skins.removeOne("Breeze/Light");
#elif defined(Q_OS_MAC)
    m_skins.removeOne("QDarkStyle");
#endif

    ui->comboBox_skin->insertItems(0, m_skins);
}

void Settings::setupLocalMoneroFrontendCombobox() {
    ui->combo_localMoneroFrontend->addItem("localmonero.co", "https://localmonero.co");
    ui->combo_localMoneroFrontend->addItem("localmonero.co/nojs", "https://localmonero.co/nojs");
    ui->combo_localMoneroFrontend->addItem("nehdddktmhvqklsnkjqcbpmb63htee2iznpcbs5tgzctipxykpj6yrid.onion",
                                           "http://nehdddktmhvqklsnkjqcbpmb63htee2iznpcbs5tgzctipxykpj6yrid.onion");

    ui->combo_localMoneroFrontend->setCurrentIndex(ui->combo_localMoneroFrontend->findData(config()->get(Config::localMoneroFrontend).toString()));
}

void Settings::enableWebsocket(bool enabled) {
    if (enabled && !config()->get(Config::offlineMode).toBool() && !config()->get(Config::disableWebsocket).toBool()) {
        websocketNotifier()->websocketClient.restart();
    } else {
        websocketNotifier()->websocketClient.stop();
    }
    ui->nodeWidget->onWebsocketStatusChanged();
    emit websocketStatusChanged(enabled);
}

Settings::~Settings() = default;