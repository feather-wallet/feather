// SPDX-License-Identifier: BSD-3-Clause
// SPDX-FileCopyrightText: 2020-2022 The Monero Project

#include "SettingsDialog.h"
#include "ui_SettingsDialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include "Icons.h"

Settings::Settings(QSharedPointer<AppContext> ctx, QWidget *parent)
        : QDialog(parent)
        , ui(new Ui::Settings)
        , m_ctx(std::move(ctx))
{
    ui->setupUi(this);

    this->setWindowIcon(QIcon("://assets/images/appicons/64x64.png"));

    ui->tabWidget->setTabVisible(3, false);
    ui->tabWidget->setTabVisible(6, false);

    connect(ui->btnCopyToClipboard, &QPushButton::clicked, this, &Settings::copyToClipboard);
    connect(ui->checkBox_multiBroadcast, &QCheckBox::toggled, [](bool toggled){
        config()->set(Config::multiBroadcast, toggled);
    });
    connect(ui->btn_multiBroadcast, &QPushButton::clicked, [this]{
       QMessageBox::information(this, "Multibroadcasting", "Multibroadcasting relays outgoing transactions to all nodes in your selected node list. This may improve transaction relay speed and reduces the chance of your transaction failing.");
    });
    connect(ui->checkBox_externalLink, &QCheckBox::clicked, this, &Settings::checkboxExternalLinkWarn);
    connect(ui->checkBox_hideBalance, &QCheckBox::toggled, [this](bool toggled){
        config()->set(Config::hideBalance, toggled);
        m_ctx->updateBalance();
    });
    connect(ui->checkBox_disableLogging, &QCheckBox::toggled, [this](bool toggled){
       config()->set(Config::disableLogging, toggled);
       WalletManager::instance()->setLogLevel(toggled ? -1 : config()->get(Config::logLevel).toInt());
    });
    connect(ui->checkBox_inactivityLockTimeout, &QCheckBox::toggled, [](bool toggled){
        config()->set(Config::inactivityLockEnabled, toggled);
    });
    connect(ui->spinBox_inactivityLockTimeout, QOverload<int>::of(&QSpinBox::valueChanged), [](int value){
        config()->set(Config::inactivityLockTimeout, value);
    });

    connect(ui->closeButton, &QDialogButtonBox::accepted, this, &Settings::close);

    // nodes
    ui->nodeWidget->setupUI(m_ctx);
    connect(ui->nodeWidget, &NodeWidget::nodeSourceChanged, m_ctx->nodes, &Nodes::onNodeSourceChanged);
    connect(ui->nodeWidget, &NodeWidget::connectToNode, m_ctx->nodes, QOverload<const FeatherNode&>::of(&Nodes::connectToNode));

    // setup checkboxes
    ui->checkBox_multiBroadcast->setChecked(config()->get(Config::multiBroadcast).toBool());
    ui->checkBox_externalLink->setChecked(config()->get(Config::warnOnExternalLink).toBool());
    ui->checkBox_hideBalance->setChecked(config()->get(Config::hideBalance).toBool());
    ui->checkBox_disableLogging->setChecked(config()->get(Config::disableLogging).toBool());
    ui->checkBox_inactivityLockTimeout->setChecked(config()->get(Config::inactivityLockEnabled).toBool());
    ui->spinBox_inactivityLockTimeout->setValue(config()->get(Config::inactivityLockTimeout).toInt());

    // setup comboboxes
    this->setupSkinCombobox();
    auto settingsSkin = config()->get(Config::skin).toString();
    if (m_skins.contains(settingsSkin))
        ui->comboBox_skin->setCurrentIndex(m_skins.indexOf(settingsSkin));

    for (int i = 0; i <= 12; i++) {
        ui->comboBox_amountPrecision->addItem(QString::number(i));
    }
    ui->comboBox_amountPrecision->setCurrentIndex(config()->get(Config::amountPrecision).toInt());

    // Date format combobox
    QDateTime now = QDateTime::currentDateTime();
    for (const auto & format : m_dateFormats) {
        ui->comboBox_dateFormat->addItem(now.toString(format));
    }
    QString dateFormatSetting = config()->get(Config::dateFormat).toString();
    if (m_dateFormats.contains(dateFormatSetting))
        ui->comboBox_dateFormat->setCurrentIndex(m_dateFormats.indexOf(dateFormatSetting));

    // Time format combobox
    for (const auto & format : m_timeFormats) {
        ui->comboBox_timeFormat->addItem(now.toString(format));
    }
    QString timeFormatSetting = config()->get(Config::timeFormat).toString();
    if (m_timeFormats.contains(timeFormatSetting))
        ui->comboBox_timeFormat->setCurrentIndex(m_timeFormats.indexOf(timeFormatSetting));

    connect(ui->comboBox_skin, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_skinChanged);
    connect(ui->comboBox_amountPrecision, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_amountPrecisionChanged);
    connect(ui->comboBox_dateFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_dateFormatChanged);
    connect(ui->comboBox_timeFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_timeFormatChanged);

    // Balance display combobox
    ui->comboBox_balanceDisplay->setCurrentIndex(config()->get(Config::balanceDisplay).toInt());
    connect(ui->comboBox_balanceDisplay, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_balanceDisplayChanged);

    // Preferred fiat currency combobox
    QStringList fiatCurrencies;
    for (int index = 0; index < ui->comboBox_fiatCurrency->count(); index++) {
        fiatCurrencies << ui->comboBox_fiatCurrency->itemText(index);
    }

    auto preferredFiatCurrency = config()->get(Config::preferredFiatCurrency).toString();
    if(!preferredFiatCurrency.isEmpty())
        if(fiatCurrencies.contains(preferredFiatCurrency))
            ui->comboBox_fiatCurrency->setCurrentText(preferredFiatCurrency);

    connect(ui->comboBox_fiatCurrency, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::fiatCurrencySelected);

    // setup paths tab
    this->updatePaths();
    connect(ui->btn_browseDefaultWalletDir, &QPushButton::clicked, [this]{
        QString walletDirOld = config()->get(Config::walletDirectory).toString();
        QString walletDir = QFileDialog::getExistingDirectory(this, "Select wallet directory ", walletDirOld, QFileDialog::ShowDirsOnly);
        if (walletDir.isEmpty())
            return;
        config()->set(Config::walletDirectory, walletDir);
        ui->lineEdit_defaultWalletDir->setText(walletDir);
    });

    // Links tab
    this->setupLocalMoneroFrontendCombobox();
    connect(ui->combo_blockExplorer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_blockExplorerChanged);
    connect(ui->combo_redditFrontend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_redditFrontendChanged);
    connect(ui->combo_localMoneroFrontend, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Settings::comboBox_localMoneroFrontendChanged);

    ui->combo_blockExplorer->setCurrentIndex(ui->combo_blockExplorer->findText(config()->get(Config::blockExplorer).toString()));
    ui->combo_redditFrontend->setCurrentIndex(ui->combo_redditFrontend->findText(config()->get(Config::redditFrontend).toString()));

    this->adjustSize();
}

void Settings::updatePaths() {
    ui->lineEdit_defaultWalletDir->setText(config()->get(Config::walletDirectory).toString());
    ui->lineEdit_configDir->setText(Config::defaultConfigDir().path());
    ui->lineEdit_applicationDir->setText(QCoreApplication::applicationDirPath());
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

void Settings::copyToClipboard() {
    ui->textLogs->copy();
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

Settings::~Settings() = default;