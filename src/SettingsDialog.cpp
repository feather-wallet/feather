// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2024 The Monero Project

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

#include "libwalletqt/WalletManager.h"
#include "utils/AppData.h"
#include "utils/Icons.h"
#include "utils/nodes.h"
#include "utils/WebsocketNotifier.h"
#include "widgets/NetworkProxyWidget.h"
#include "utils/ColorScheme.h"

Settings::Settings(Nodes *nodes, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::Settings)
        , m_nodes(nodes)
{
    ui->setupUi(this);

    this->setupAppearanceTab();
    this->setupNetworkTab();
    this->setupStorageTab();
    this->setupDisplayTab();
    this->setupMemoryTab();
    this->setupTransactionsTab();
    this->setupPluginsTab();
    this->setupMiscTab();

    connect(ui->selector, &QListWidget::currentItemChanged, [this](QListWidgetItem *current, QListWidgetItem *previous){
        ui->stackedWidget->setCurrentIndex(current->type());
    });

    ui->selector->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->selector->setSelectionBehavior(QAbstractItemView::SelectRows);

    new QListWidgetItem(icons()->icon("interface_32px.png"), "Appearance", ui->selector, Pages::APPEARANCE);
    new QListWidgetItem(icons()->icon("nw_32px.png"), "Network", ui->selector, Pages::NETWORK);
    new QListWidgetItem(icons()->icon("hd_32px.png"), "Storage", ui->selector, Pages::STORAGE);
    new QListWidgetItem(icons()->icon("vrdp_32px.png"), "Display", ui->selector, Pages::DISPLAY);
//  new QListWidgetItem(icons()->icon("chipset_32px.png"), "Memory", ui->selector, Pages::MEMORY);
    new QListWidgetItem(icons()->icon("file_manager_32px.png"), "Transactions", ui->selector, Pages::TRANSACTIONS);
    QString connectIcon = ColorScheme::darkScheme ? "connect_white.svg" : "connect.svg";;
    new QListWidgetItem(icons()->icon(connectIcon), "Plugins", ui->selector, Pages::PLUGINS);
    new QListWidgetItem(icons()->icon("settings_disabled_32px.png"), "Misc", ui->selector, Pages::MISC);

    ui->selector->setFixedWidth(ui->selector->sizeHintForColumn(0) + ui->selector->frameWidth() + 5);

//    for (int i = 0; i < ui->selector->count(); ++i) {
//        QListWidgetItem *item = ui->selector->item(i);
//        item->setSizeHint(QSize(item->sizeHint().width(), ui->selector->iconSize().height() + 8));
//    }

    connect(ui->closeButton, &QDialogButtonBox::accepted, [this]{
        // Save Proxy settings
        bool isProxySettingChanged = ui->proxyWidget->isProxySettingsChanged();
        ui->proxyWidget->setProxySettings();
        if (isProxySettingChanged) {
            emit proxySettingsChanged();
        }

        conf()->set(Config::lastSettingsPage, ui->selector->currentRow());
        this->close();
    });

    this->setSelection(conf()->get(Config::lastSettingsPage).toInt());

    this->adjustSize();
}

void Settings::setupAppearanceTab() {
    // [Theme]
    this->setupThemeComboBox();
    auto settingsTheme = conf()->get(Config::skin).toString();
    if (m_themes.contains(settingsTheme)) {
        ui->comboBox_theme->setCurrentIndex(m_themes.indexOf(settingsTheme));
    }
    connect(ui->comboBox_theme, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int pos){
        emit skinChanged(m_themes.at(pos));
    });

    // [Amount precision]
    for (int i = 0; i <= 12; i++) {
        ui->comboBox_amountPrecision->addItem(QString::number(i));
    }
    ui->comboBox_amountPrecision->setCurrentIndex(conf()->get(Config::amountPrecision).toInt());

    connect(ui->comboBox_amountPrecision, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int pos){
        conf()->set(Config::amountPrecision, pos);
        emit updateBalance();
    });

    // [Date format]
    QDateTime now = QDateTime::currentDateTime();
    for (const auto & format : m_dateFormats) {
        ui->comboBox_dateFormat->addItem(now.toString(format));
    }
    QString dateFormatSetting = conf()->get(Config::dateFormat).toString();
    if (m_dateFormats.contains(dateFormatSetting)) {
        ui->comboBox_dateFormat->setCurrentIndex(m_dateFormats.indexOf(dateFormatSetting));
    }

    connect(ui->comboBox_dateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int pos){
        conf()->set(Config::dateFormat, m_dateFormats.at(pos));
    });

    // [Time format]
    for (const auto & format : m_timeFormats) {
        ui->comboBox_timeFormat->addItem(now.toString(format));
    }
    QString timeFormatSetting = conf()->get(Config::timeFormat).toString();
    if (m_timeFormats.contains(timeFormatSetting)) {
        ui->comboBox_timeFormat->setCurrentIndex(m_timeFormats.indexOf(timeFormatSetting));
    }

    connect(ui->comboBox_timeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int pos){
        conf()->set(Config::timeFormat, m_timeFormats.at(pos));
    });


    // [Balance display]
    ui->comboBox_balanceDisplay->setCurrentIndex(conf()->get(Config::balanceDisplay).toInt());
    connect(ui->comboBox_balanceDisplay, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int pos){
        conf()->set(Config::balanceDisplay, pos);
        emit updateBalance();
    });

    // [Preferred fiat currency]
    QStringList availableFiatCurrencies = appData()->prices.rates.keys();
    for (const auto &currency : availableFiatCurrencies) {
        ui->comboBox_fiatCurrency->addItem(currency);
    }

    QStringList fiatCurrencies;
    for (int index = 0; index < ui->comboBox_fiatCurrency->count(); index++) {
        fiatCurrencies << ui->comboBox_fiatCurrency->itemText(index);
    }

    auto preferredFiatCurrency = conf()->get(Config::preferredFiatCurrency).toString();
    if (!preferredFiatCurrency.isEmpty() && fiatCurrencies.contains(preferredFiatCurrency)) {
        ui->comboBox_fiatCurrency->setCurrentText(preferredFiatCurrency);
    }

    connect(ui->comboBox_fiatCurrency, QOverload<int>::of(&QComboBox::currentIndexChanged), [this](int index){
        QString selection = ui->comboBox_fiatCurrency->itemText(index);
        conf()->set(Config::preferredFiatCurrency, selection);
        emit preferredFiatCurrencyChanged(selection);
    });
}

void Settings::setupNetworkTab() {
    // Node
    if (m_nodes) {
        ui->nodeWidget->setupUI(m_nodes);
        connect(ui->nodeWidget, &NodeWidget::nodeSourceChanged, m_nodes, &Nodes::onNodeSourceChanged);
        connect(ui->nodeWidget, &NodeWidget::connectToNode, m_nodes, QOverload<const FeatherNode&>::of(&Nodes::connectToNode));
    } else {
        m_nodes = new Nodes(this, nullptr);
        ui->nodeWidget->setupUI(m_nodes);
        ui->nodeWidget->setCanConnect(false);
    }

    // Proxy
    connect(ui->proxyWidget, &NetworkProxyWidget::proxySettingsChanged, this, &Settings::onProxySettingsChanged);

    // Websocket
    // [Obtain third-party data]
    ui->checkBox_enableWebsocket->setChecked(!conf()->get(Config::disableWebsocket).toBool());
    connect(ui->checkBox_enableWebsocket, &QCheckBox::toggled, [this](bool checked){
        conf()->set(Config::disableWebsocket, !checked);
        this->enableWebsocket(checked);
    });

    // Overview
    ui->checkBox_offlineMode->setChecked(conf()->get(Config::offlineMode).toBool());
    connect(ui->checkBox_offlineMode, &QCheckBox::toggled, [this](bool checked){
        conf()->set(Config::offlineMode, checked);
        emit offlineMode(checked);
        this->enableWebsocket(!checked);
    });
}

void Settings::setupStorageTab() {
    // Paths
    ui->lineEdit_defaultWalletDir->setText(conf()->get(Config::walletDirectory).toString());
    ui->lineEdit_configDir->setText(Config::defaultConfigDir().path());
    ui->lineEdit_applicationDir->setText(Utils::applicationPath());

    bool portableMode = Utils::isPortableMode();
    if (portableMode) {
        ui->btn_browseDefaultWalletDir->setDisabled(true);
        ui->btn_browseDefaultWalletDir->setToolTip("Portable Mode enabled");
    }

    connect(ui->btn_browseDefaultWalletDir, &QPushButton::clicked, [this]{
        QString walletDirOld = conf()->get(Config::walletDirectory).toString();
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", walletDirOld, QFileDialog::ShowDirsOnly);
        if (walletDir.isEmpty())
            return;
        conf()->set(Config::walletDirectory, walletDir);
        ui->lineEdit_defaultWalletDir->setText(walletDir);
    });

    connect(ui->btn_openWalletDir, &QPushButton::clicked, []{
        QDesktopServices::openUrl(QUrl::fromLocalFile(conf()->get(Config::walletDirectory).toString()));
    });

    connect(ui->btn_openConfigDir, &QPushButton::clicked, []{
        QDesktopServices::openUrl(QUrl::fromLocalFile(Config::defaultConfigDir().path()));
    });

    ui->frame_portableMode->setVisible(portableMode);

    // Logging
    // [Write log files to disk]
    ui->checkBox_enableLogging->setChecked(!conf()->get(Config::disableLogging).toBool());
    connect(ui->checkBox_enableLogging, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::disableLogging, !toggled);
        WalletManager::instance()->setLogLevel(toggled ? conf()->get(Config::logLevel).toInt() : -1);
    });

    // [Log level]
    ui->comboBox_logLevel->setCurrentIndex(conf()->get(Config::logLevel).toInt());
    connect(ui->comboBox_logLevel, QOverload<int>::of(&QComboBox::currentIndexChanged), [](int index){
       conf()->set(Config::logLevel, index);
       if (!conf()->get(Config::disableLogging).toBool()) {
           WalletManager::instance()->setLogLevel(index);
       }
    });

    // [Write stack trace to disk on crash]
    ui->checkBox_writeStackTraceToDisk->setChecked(conf()->get(Config::writeStackTraceToDisk).toBool());
    connect(ui->checkBox_writeStackTraceToDisk, &QCheckBox::toggled, [](bool toggled){
       conf()->set(Config::writeStackTraceToDisk, toggled);
    });

    // [Open log file]
    connect(ui->btn_openLogFile, &QPushButton::clicked, []{
        QDesktopServices::openUrl(QUrl::fromLocalFile(Config::defaultConfigDir().path() + "/libwallet.log"));
    });

    // Misc
    // [Save recently opened wallet to config file]
    ui->checkBox_writeRecentlyOpened->setChecked(conf()->get(Config::writeRecentlyOpenedWallets).toBool());
    connect(ui->checkBox_writeRecentlyOpened, &QCheckBox::toggled, [](bool toggled){
       conf()->set(Config::writeRecentlyOpenedWallets, toggled);
       if (!toggled) {
           conf()->set(Config::recentlyOpenedWallets, {});
       }
    });
}

void Settings::setupDisplayTab() {
    // [Hide balance]
    ui->checkBox_hideBalance->setChecked(conf()->get(Config::hideBalance).toBool());
    connect(ui->checkBox_hideBalance, &QCheckBox::toggled, [this](bool toggled){
        conf()->set(Config::hideBalance, toggled);
        emit updateBalance();
    });

    // [Hide update notifications]
    ui->checkBox_hideUpdateNotifications->setChecked(conf()->get(Config::hideUpdateNotifications).toBool());
    connect(ui->checkBox_hideUpdateNotifications, &QCheckBox::toggled, [this](bool toggled){
        conf()->set(Config::hideUpdateNotifications, toggled);
        emit hideUpdateNotifications(toggled);
    });

    // [Hide transaction notifications]
    ui->checkBox_hideTransactionNotifications->setChecked(conf()->get(Config::hideNotifications).toBool());
    connect(ui->checkBox_hideTransactionNotifications, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::hideNotifications, toggled);
    });

    // [Warn before opening external link]
    ui->checkBox_warnOnExternalLink->setChecked(conf()->get(Config::warnOnExternalLink).toBool());
    connect(ui->checkBox_warnOnExternalLink, &QCheckBox::clicked, this, [this]{
        bool state = ui->checkBox_warnOnExternalLink->isChecked();
        conf()->set(Config::warnOnExternalLink, state);
    });

    // [Lock wallet on inactivity]
    ui->checkBox_lockOnInactivity->setChecked(conf()->get(Config::inactivityLockEnabled).toBool());
    ui->spinBox_lockOnInactivity->setValue(conf()->get(Config::inactivityLockTimeout).toInt());
    connect(ui->checkBox_lockOnInactivity, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::inactivityLockEnabled, toggled);
    });
    connect(ui->spinBox_lockOnInactivity, QOverload<int>::of(&QSpinBox::valueChanged), [](int value){
        conf()->set(Config::inactivityLockTimeout, value);
    });

    // [Lock wallet on minimize]
    ui->checkBox_lockOnMinimize->setChecked(conf()->get(Config::lockOnMinimize).toBool());
    connect(ui->checkBox_lockOnMinimize, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::lockOnMinimize, toggled);
    });
}

void Settings::setupMemoryTab() {
    // Nothing here, yet
}

void Settings::setupTransactionsTab() {
    // [Multibroadcast outgoing transactions]
    ui->checkBox_multibroadcast->setChecked(conf()->get(Config::multiBroadcast).toBool());
    connect(ui->checkBox_multibroadcast, &QCheckBox::toggled, [](bool toggled){
        conf()->set(Config::multiBroadcast, toggled);
    });
    connect(ui->btn_multibroadcast, &QPushButton::clicked, [this]{
        Utils::showInfo(this, "Multibroadcasting", "Multibroadcasting relays outgoing transactions to all nodes in your node list. This may improve transaction relay speed and reduces the chance of your transaction failing.");
    });

    // Hide unimplemented settings
    ui->checkBox_alwaysOpenAdvancedTxDialog->hide();
    ui->checkBox_requirePasswordToSpend->hide();

    // [Manual fee-tier selection]
    ui->checkBox_manualFeeTierSelection->setChecked(conf()->get(Config::manualFeeTierSelection).toBool());
    connect(ui->checkBox_manualFeeTierSelection, &QCheckBox::toggled, [this](bool toggled){
        if (toggled) {
            auto result = QMessageBox::question(this, "Privacy warning", "Using a non-automatic fee makes your transactions stick out and harms your privacy.\n\nAre you sure you want to enable manual fee-tier selection?");
            if (result == QMessageBox::No) {
                ui->checkBox_manualFeeTierSelection->setChecked(false);
                return;
            }

        }

        conf()->set(Config::manualFeeTierSelection, toggled);
        emit manualFeeSelectionEnabled(toggled);
    });

    ui->checkBox_subtractFeeFromAmount->setChecked(conf()->get(Config::subtractFeeFromAmount).toBool());
    connect(ui->checkBox_subtractFeeFromAmount, &QCheckBox::toggled, [this](bool toggled){
        conf()->set(Config::subtractFeeFromAmount, toggled);
        emit subtractFeeFromAmountEnabled(toggled);
    });
}

void Settings::setupPluginsTab() {
    connect(ui->pluginWidget, &PluginWidget::pluginConfigured, [this](const QString &id) {
       emit pluginConfigured(id);
    });
}

void Settings::setupMiscTab() {
    // [Block explorer]
    ui->blockExplorerConfigureWidget->setup("Block explorers", Config::blockExplorers, Config::blockExplorer, {"%txid%"});
}

void Settings::onProxySettingsChanged() {
    ui->closeButton->addButton(QDialogButtonBox::Apply);
    connect(ui->closeButton->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, [this](){
        ui->proxyWidget->setProxySettings();
        m_nodes->updateModels();
        emit proxySettingsChanged();
        ui->closeButton->removeButton(ui->closeButton->button(QDialogButtonBox::Apply));
    });
}

void Settings::showNetworkProxyTab() {
    this->setSelection(Settings::Pages::NETWORK);
    ui->tabWidget_network->setCurrentIndex(1);
}

void Settings::setupThemeComboBox() {
#if defined(Q_OS_WIN)
    m_themes.removeOne("Breeze/Dark");
    m_themes.removeOne("Breeze/Light");
#elif defined(Q_OS_MAC)
    m_themes.removeOne("QDarkStyle");
#endif

    ui->comboBox_theme->insertItems(0, m_themes);
}

void Settings::setSelection(int index) {
    ui->selector->setCurrentRow(index);
}

void Settings::enableWebsocket(bool enabled) {
    if (enabled && !conf()->get(Config::offlineMode).toBool() && !conf()->get(Config::disableWebsocket).toBool()) {
        websocketNotifier()->websocketClient->restart();
    } else {
        websocketNotifier()->websocketClient->stop();
    }
    ui->nodeWidget->onWebsocketStatusChanged();
    emit websocketStatusChanged(enabled);
}

Settings::~Settings() = default;